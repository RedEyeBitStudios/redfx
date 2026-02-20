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
				using ExtensionsSet = Subsystems::ResourcesManagerRoot::ResourceManifest::Extensions;

				if constexpr(std::is_same_v<T, ExtensionsSet::Extension_Font>)
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

						//vkCmdPipelineBarrier(selected_unit->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
						virtual_staging_buf += mem_block_info.resources[block_index].bytes_offset;
						current_staging_offset += mem_block_info.resources[block_index].bytes_offset;
						block_index++;
					}
					this->memory_manager_data.assets_gpu[asset_name.data()] = std::move(gpgpu_cache);
				}
				else if constexpr(std::is_same_v<T, ExtensionsSet::Extension_RedFXUI>)
				{
					return;
				}
				else
				{
					static_assert(false, "Unimplemented branch.");
				}
			},
			resource->manifest_ptr->extension
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