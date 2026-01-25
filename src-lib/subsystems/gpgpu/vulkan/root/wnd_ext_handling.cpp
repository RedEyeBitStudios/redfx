#include <internals/subsystems/gpgpu/root_vulkan.hpp>
#include <internals/subsystems/gpgpu/vulkan/window_extension.hpp>
#include <subsystems.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_RootVulkan;

void ClassImpl::clearWindowExtension(GPGPU_WindowExtension* ext)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(ext);
	const auto primary_commons = this->primary_dvc->getCommons();
	vkQueueWaitIdle(this->primary_dvc->getQueue(GPGPU_Device_Vulkan::queue_name_graphics).handle);

	constexpr const auto cmd_allocations_size = sizeof(wnd_ext->cmd_allocations) / sizeof(VkCommandPool);
	for (auto& cmd_allocation : std::span(reinterpret_cast<VkCommandPool*>(&wnd_ext->cmd_allocations), cmd_allocations_size))
	{
		vkDestroyCommandPool(primary_commons.dvc, cmd_allocation, nullptr);
	}

	for (auto& f : wnd_ext->frames.presentation->frames)
	{
		vkDestroyImageView(primary_commons.dvc, f.presentation_buf_view, nullptr);
		vkDestroySemaphore(primary_commons.dvc, f.cmd_semaphore, nullptr);
		vkDestroyFence(primary_commons.dvc, f.cmd_fence, nullptr);
	}

	for (auto& f : wnd_ext->frames.ui->frames)
	{
		vkDestroyImageView(primary_commons.dvc, f.color_framebuffer_view, nullptr);
		vkDestroyImage(primary_commons.dvc, f.color_framebuffer, nullptr);
		vkDestroyFence(primary_commons.dvc, f.cmd_fence, nullptr);
	}

	vkDestroySemaphore(primary_commons.dvc, wnd_ext->acquire_semaphore, nullptr);
	vkDestroySemaphore(primary_commons.dvc, wnd_ext->ui_render_semaphore, nullptr);
	vkDestroySwapchainKHR(primary_commons.dvc, wnd_ext->swp, nullptr);
	vkDestroySurfaceKHR(this->sys_con, wnd_ext->surface, nullptr);
}
void ClassImpl::makeWindowExtension(GPGPU_WindowExtension* ext)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(ext);
	const auto commons = this->primary_dvc->getCommons();
	const auto queue = this->primary_dvc->getQueue(GPGPU_Device_Vulkan::queue_name_graphics);

	const VkCommandPoolCreateInfo cmd_pool_info
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queue.queue_family_index
	};

	vkCreateCommandPool(commons.dvc, &cmd_pool_info, nullptr, &wnd_ext->cmd_allocations.present);
	vkCreateCommandPool(commons.dvc, &cmd_pool_info, nullptr, &wnd_ext->cmd_allocations.ui);
	vkCreateCommandPool(commons.dvc, &cmd_pool_info, nullptr, &wnd_ext->cmd_allocations.gfx);

	const VkSemaphoreCreateInfo semaphore_info
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr
	};
	const VkFenceCreateInfo fence_info
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	vkCreateSemaphore(commons.dvc, &semaphore_info, nullptr, &wnd_ext->acquire_semaphore);
	vkCreateSemaphore(commons.dvc, &semaphore_info, nullptr, &wnd_ext->ui_render_semaphore);

	for (auto& f : wnd_ext->frames.presentation->frames)
	{
		vkCreateFence(commons.dvc, &fence_info, nullptr, &f.cmd_fence);
		vkCreateSemaphore(commons.dvc, &semaphore_info, nullptr, &f.cmd_semaphore);

		const VkCommandBufferAllocateInfo allocation_info
		{	
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = wnd_ext->cmd_allocations.present,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
	 	};
		vkAllocateCommandBuffers(commons.dvc, &allocation_info, &f.cmd);
	}

	for (auto& f : wnd_ext->frames.ui->frames)
	{
		const VkCommandBufferAllocateInfo ui_cmd_bufs_info
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = wnd_ext->cmd_allocations.ui,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};
		vkAllocateCommandBuffers(commons.dvc, &ui_cmd_bufs_info, &f.cmd);
		vkCreateFence(commons.dvc, &fence_info, nullptr, &f.cmd_fence);
	}
}
void ClassImpl::recreateSwapchain(VidRoot::VidWindows::VidWndInfo& info, GPGPU_WindowExtension_Vulkan* wnd_ext)
{
	// TODO: Make sure everything are recreated as a swapchain itself is.
	vkQueueWaitIdle(this->primary_dvc->getQueue(GPGPU_Device_Vulkan::queue_name_graphics).handle);

	const auto swp_capabilities = this->primary_dvc->getSwapchainCapabilities(wnd_ext->surface);
	const auto commons = this->primary_dvc->getCommons();

	// Clear before recreation.
	if (wnd_ext->swp != VK_NULL_HANDLE)
	{
		for (auto& frame : wnd_ext->frames.presentation->frames)
		{
			vkDestroyImageView(commons.dvc, frame.presentation_buf_view, nullptr);
		}
		vkDestroySwapchainKHR(commons.dvc, wnd_ext->swp, nullptr);
		for(auto& frame : wnd_ext->frames.ui->frames)
		{
			vkDestroyImageView(commons.dvc, frame.color_framebuffer_view, nullptr);
			vkDestroyImage(commons.dvc, frame.color_framebuffer, nullptr);
			vkDestroyFence(commons.dvc, frame.cmd_fence, nullptr);
		}
		this->primary_dvc->requestDeallocation(wnd_ext->allocations.ui.resizable_memory_blocks);

		wnd_ext->frames.presentation.reset();
		wnd_ext->frames.ui.reset();

		vkResetCommandPool(commons.dvc, wnd_ext->cmd_allocations.present, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		vkResetCommandPool(commons.dvc, wnd_ext->cmd_allocations.ui, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}

	// Create.
	const VkSwapchainCreateInfoKHR swp_info
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.surface = wnd_ext->surface,
		.minImageCount = swp_capabilities.capabilities.minImageCount,
		.imageFormat = swp_capabilities.format.format,
		.imageColorSpace = swp_capabilities.format.colorSpace,
		.imageExtent
		{
			.width = info.mode.wh.x,
			.height = info.mode.wh.y
		},
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
		.clipped = VK_TRUE,
		.oldSwapchain = nullptr
	};

	if (const auto result = vkCreateSwapchainKHR(commons.dvc, &swp_info, nullptr, &wnd_ext->swp); result != VK_SUCCESS)
	{
		nxcraft::Subsystems::LogRoot::Message(this, std::format("Swapchain creation failed: {}", static_cast<int>(result)), nxcraft::Subsystems::LogRoot::Message::Flags::MARK_AS_CRITICAL_ERROR);
	}

	const auto swp_images = [&commons, wnd_ext]() -> std::vector<VkImage>
	{
		uint32_t swp_image_count = 0;
		vkGetSwapchainImagesKHR(commons.dvc, wnd_ext->swp, &swp_image_count, nullptr);
		std::vector<VkImage> images(swp_image_count);
		vkGetSwapchainImagesKHR(commons.dvc, wnd_ext->swp, &swp_image_count, images.data());
		return images;
	}();

	wnd_ext->frames.ui = std::make_unique<FramesDataUnit_Vulkan<GPGPU_FrameData_Vulkan_UI>>(GPGPU_WindowExtension_Vulkan::ui_frames_count);
	wnd_ext->frames.presentation = std::make_unique<FramesDataUnit_Vulkan<GPGPU_FrameData_Vulkan_Present>>(swp_images.size());
	//wnd_ext->frames.gfx.frames.resize(5);

	for (auto i = 0; i < swp_images.size(); i++)
	{
		const VkImageViewCreateInfo info
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = swp_images[i],
			.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
			.format = swp_capabilities.format.format,
			.components
			{
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		vkCreateImageView(commons.dvc, &info, nullptr, &wnd_ext->frames.presentation->frames[i].presentation_buf_view);
		wnd_ext->frames.presentation->frames[i].presentation_buffer = swp_images[i];
	}

	std::vector<VkImage*> ui_frames;
	constexpr const VkFormat ui_frame_format = VK_FORMAT_R8G8B8A8_UNORM;
	for (auto& frame : wnd_ext->frames.ui->frames)
	{
		const VkImageCreateInfo image_create_info
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = ui_frame_format,
			.extent = 
			{
				.width = info.mode.wh.x,
				.height = info.mode.wh.y,
				.depth = 1
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		vkCreateImage(commons.dvc, &image_create_info, nullptr, &frame.color_framebuffer);
		ui_frames.push_back(&frame.color_framebuffer);
	}
	wnd_ext->allocations.ui.resizable_memory_blocks = this->primary_dvc->allocate(ui_frames, {}, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	for (auto i = 0; i < ui_frames.size(); i++)
	{
		const VkImageViewCreateInfo info
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = *ui_frames[i],
			.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
			.format = ui_frame_format,
			.components
			{
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		vkCreateImageView(commons.dvc, &info, nullptr, &wnd_ext->frames.ui->frames[i].color_framebuffer_view);
	}
}