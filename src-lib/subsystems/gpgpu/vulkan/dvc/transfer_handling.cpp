#include <internals/subsystems/gpgpu/vulkan/devices.hpp>
#include <subsystems.hpp>
#include <algorithm>
#include <type_traits>


using ClassImpl = nxcraft::intern::subsystems::GPGPU_Device_Vulkan;

ClassImpl::GPGPU_Resource* ClassImpl::retrieveResource(std::string_view asset_name)
{
	for (auto& unit : this->transfer.transfer_units)
	{
		if (std::ranges::contains(unit->assets_to_transfer, asset_name))
		{
			return nullptr;
		}
	}
	if (this->memory_manager_data.assets_gpu.contains(asset_name.data()))
	{
		return &this->memory_manager_data.assets_gpu[asset_name.data()];
	}
	return nullptr;
}

void ClassImpl::addTransferQueue(std::string_view asset_name)
{
	for (auto& unit : this->transfer.transfer_units)
	{
		if (std::ranges::contains(unit->assets_to_transfer, asset_name))
		{
			return;
		}
	}
	if (Subsystems::getSubsystem_ResourcesManager().retrieveResourceView(asset_name)->data.get() && !std::ranges::contains(this->transfer.asset_queue, asset_name))
	{
		nxcraft::Subsystems::LogRoot::Message(&this->transfer, std::format("Asset '{}' appended into GPGPU queue.", asset_name));
		this->transfer.asset_queue.push_back(asset_name);
	}
	else
	{
		Subsystems::getSubsystem_ResourcesManager().appendAsynchronousQueue(asset_name);
	}
}
void ClassImpl::waitForTransfer(AsyncUploadUnit* unit)
{
	vkWaitForFences(this->dvc, 1, &unit->fence, true, UINT64_MAX);

}
void ClassImpl::submitTransfer()
{
	for (auto& unit : this->transfer.transfer_units)
	{
		if (vkGetFenceStatus(this->dvc, unit->fence) == VK_SUCCESS)
		{
			unit->assets_to_transfer.clear();
			vkResetCommandBuffer(unit->cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		}
	}

	if (this->transfer.asset_queue.empty()) return;
	// TODO: All further instructions should be done asynchronously on separated thread.
	nxcraft::Subsystems::LogRoot::Message(&this->transfer, std::format("Searching for available upload unit..."));
	AsyncUploadUnit* selected_unit = nullptr;
	
	while (!selected_unit)
	{
		for (auto& unit : this->transfer.transfer_units)
		{
			if (vkWaitForFences(this->dvc, 1, &unit->fence, false, 500) == VK_SUCCESS)
			{
				selected_unit = unit.get();
				break;
			}
		}
	}

	nxcraft::Subsystems::LogRoot::Message(&this->transfer, std::format("Found {}. Submitting...", reinterpret_cast<void*>(selected_unit)));

	const VkCommandBufferBeginInfo begin_info
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = nullptr
	};

	vkBeginCommandBuffer(selected_unit->cmd, &begin_info);

	char* virtual_staging_buf = nullptr;
	vkMapMemory(this->dvc, selected_unit->buffer_mem, 0, VK_WHOLE_SIZE, 0, reinterpret_cast<void**>(&virtual_staging_buf));
	uint64_t current_staging_offset = 0; // TODO: Secure staging buffer against memory overflow.
	for (auto asset_name : this->transfer.asset_queue)
	{
		auto resource = Subsystems::getSubsystem_ResourcesManager().retrieveResourceView(asset_name);
		std::visit
		(
			[&resource, this, selected_unit, &asset_name, &virtual_staging_buf, &current_staging_offset](auto& ext)
			{
				using T = std::decay_t<decltype(ext)>;
				using ResourceManifestClasses = Subsystems::ResourcesManagerRoot::ResourceManifestClasses;

				if constexpr(std::is_same_v<T, ResourceManifestClasses::Manifest_RedFXFont>)
				{
					auto font = static_cast<Subsystems::ResourcesManagerRoot::ResourceData_Font*>(resource->data.get());
					std::vector<VkBuffer*> bufs;
					bufs.reserve(font->getCharacters().size());
					ResourceClasses::Resource_RedFX_Font gpgpu_cache;
					
					for (auto& character : font->getCharacters())
					{
						const VkBufferCreateInfo buf_info
						{
							.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
							.pNext = nullptr,
							.flags = 0,
							.size = std::span(character.second.vertices).size_bytes(),
							.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
							.queueFamilyIndexCount = 0,
							.pQueueFamilyIndices = nullptr
						};
						auto& buffer = gpgpu_cache.characters_data[character.first].v_buffer;
						gpgpu_cache.characters_data[character.first].v_count = character.second.vertices.size();
						
						vkCreateBuffer(this->dvc, &buf_info, nullptr, &buffer);
						bufs.push_back(&buffer);
					}

					gpgpu_cache.mem_block = this->allocate({}, bufs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
					auto mem_block_info = this->getMemBlockInfo(gpgpu_cache.mem_block);

					uint64_t block_index = 0;
					for (auto& character : font->getCharacters())
					{
						memcpy(virtual_staging_buf, character.second.vertices.data(), std::span(character.second.vertices).size_bytes());

						const VkBufferCopy copy_info
						{
							.srcOffset = current_staging_offset,
							.dstOffset = 0,
							.size = std::span(character.second.vertices).size_bytes()
						};

						vkCmdCopyBuffer(selected_unit->cmd, selected_unit->staging_buffer, std::get<VkBuffer>(mem_block_info.resources[block_index].res), 1, &copy_info);

						virtual_staging_buf += mem_block_info.resources[block_index].bytes_offset;
						current_staging_offset += mem_block_info.resources[block_index].bytes_offset;
						block_index++;
					}
					this->memory_manager_data.assets_gpu[asset_name.data()] = std::move(gpgpu_cache);
				}
				else if constexpr(std::is_same_v<T, ResourceManifestClasses::Manifest_RedFXUI>)
				{
					return;
				}
				else if constexpr(std::is_same_v<T, ResourceManifestClasses::Manifest_Texture>)
				{
					auto image = static_cast<Subsystems::ResourcesManagerRoot::ResourceData_Image*>(resource->data.get());
					using ImageFormat = Subsystems::ResourcesManagerRoot::ResourceData_Image::Format;
					std::vector<VkImage*> images;

					ResourceClasses::Resource_Image gpgpu_cache;

					std::unordered_map<ImageFormat, VkFormat> formats
					{
						{ ImageFormat::BW8, VK_FORMAT_R8_UNORM },
						{ ImageFormat::RGB8, VK_FORMAT_R8G8B8_UNORM },
						{ ImageFormat::RGBA8, VK_FORMAT_R8G8B8A8_UNORM }
					};

					const VkImageCreateInfo img_info
					{
						.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
						.pNext = nullptr,
						.flags = 0,
						.imageType = VK_IMAGE_TYPE_2D,
						.format = formats[image->retrieveFormat()],
						.extent = 
						{
							.width = image->retrieveResolution().x,
							.height = image->retrieveResolution().y,
							.depth = 1
						},
						.mipLevels = 1,
						.arrayLayers = 1,
						.samples = VK_SAMPLE_COUNT_1_BIT,
						.tiling = VK_IMAGE_TILING_OPTIMAL,
						.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
						.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
						.queueFamilyIndexCount = 0,
						.pQueueFamilyIndices = nullptr,
						.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED
					};

					vkCreateImage(this->dvc, &img_info, nullptr, &gpgpu_cache.image);

					gpgpu_cache.mem_block = this->allocate({ &gpgpu_cache.image }, {}, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

					memcpy(virtual_staging_buf, image->retrievePixelsData().data(), std::span(image->retrievePixelsData()).size_bytes());

					VkImageMemoryBarrier img_barrier_info
					{
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
						.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
						.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						.srcQueueFamilyIndex = this->transfer.queue.family_index,
						.dstQueueFamilyIndex = this->transfer.queue.family_index,
						.image = gpgpu_cache.image,
						.subresourceRange
						{
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1
						}
					};

					vkCmdPipelineBarrier(selected_unit->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &img_barrier_info);

					const VkBufferImageCopy copy_info
					{
						.bufferOffset = current_staging_offset,
						.bufferRowLength = {},
						.imageSubresource = 
						{
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.mipLevel = 0,
							.baseArrayLayer = 0,
							.layerCount = 1
						},
						.imageOffset = {},
						.imageExtent = img_info.extent
					};

					vkCmdCopyBufferToImage(selected_unit->cmd, selected_unit->staging_buffer, gpgpu_cache.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_info);

					img_barrier_info.dstQueueFamilyIndex = this->main_queue.queue_family_index;
					img_barrier_info.oldLayout = img_barrier_info.newLayout;
					img_barrier_info.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					vkCmdPipelineBarrier(selected_unit->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &img_barrier_info);

					current_staging_offset += this->getMemBlockInfo(gpgpu_cache.mem_block).size_summary;
					virtual_staging_buf += this->getMemBlockInfo(gpgpu_cache.mem_block).size_summary;

					const VkImageViewCreateInfo view_info
					{
						.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
						.pNext = nullptr,
						.flags = 0,
						.image = gpgpu_cache.image,
						.viewType = VK_IMAGE_VIEW_TYPE_2D,
						.format = img_info.format,
						.components = 
						{
							.r = VK_COMPONENT_SWIZZLE_IDENTITY,
							.g = VK_COMPONENT_SWIZZLE_IDENTITY,
							.b = VK_COMPONENT_SWIZZLE_IDENTITY,
							.a = VK_COMPONENT_SWIZZLE_IDENTITY
						},
						.subresourceRange = 
						{
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1
						}
					};
					vkCreateImageView(this->dvc, &view_info, nullptr, &gpgpu_cache.view);

					this->memory_manager_data.assets_gpu[asset_name.data()] = std::move(gpgpu_cache);
				}
				else
				{
					static_assert(false, "Unimplemented branch.");
				}
			},
			resource->manifest_ptr->manifest
		);
	}
	vkUnmapMemory(this->dvc, selected_unit->buffer_mem);

	vkEndCommandBuffer(selected_unit->cmd);

	vkResetFences(this->dvc, 1, &selected_unit->fence);
	selected_unit->assets_to_transfer = std::move(this->transfer.asset_queue);

	const VkSubmitInfo submit_info
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = nullptr,
		.pWaitDstStageMask = nullptr,
		.commandBufferCount = 1,
		.pCommandBuffers = &selected_unit->cmd,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = nullptr
	};
	vkQueueSubmit(this->transfer.queue.handle, 1, &submit_info, selected_unit->fence);
	nxcraft::Subsystems::LogRoot::Message(&this->transfer, std::format("Submitted {}.", reinterpret_cast<void*>(selected_unit)));
}