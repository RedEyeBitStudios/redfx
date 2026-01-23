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

	vkDestroySemaphore(primary_commons.dvc, wnd_ext->acquire_semaphore, nullptr);
	vkDestroySwapchainKHR(primary_commons.dvc, wnd_ext->swp, nullptr);
	vkDestroySurfaceKHR(this->sys_con, wnd_ext->surface, nullptr);
}
void ClassImpl::makeWindowExtension(GPGPU_WindowExtension* ext)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(ext);

	const auto swp_capabilities = this->primary_dvc->getSwapchainCapabilities(wnd_ext->surface);
	const auto commons = this->primary_dvc->getCommons();

	const VkCommandPoolCreateInfo cmd_pool_info
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = this->queue_family_index
	};
	vkCreateCommandPool(commons.dvc, &cmd_pool_info, nullptr, &wnd_ext->cmd_allocations.present);
	vkCreateCommandPool(commons.dvc, &cmd_pool_info, nullptr, &wnd_ext->cmd_allocations.ui);
	vkCreateCommandPool(commons.dvc, &cmd_pool_info, nullptr, &wnd_ext->cmd_allocations.gfx);

	wnd_ext->frames.presentation = std::make_unique<FramesDataUnit_Vulkan<FrameData_Vulkan_Present>>(swp_capabilities.capabilities.minImageCount);
	//wnd_ext->frames.ui.frames.resize(3);
	//wnd_ext->frames.gfx.frames.resize(5);
}
void ClassImpl::recreateSwapchain(VidRoot::VidWindows::VidWndInfo& info, GPGPU_WindowExtension_Vulkan* wnd_ext)
{
	// TODO: Swapchain creation.
	vkQueueWaitIdle(this->primary_dvc->getQueue(GPGPU_Device_Vulkan::queue_name_graphics).handle);

	const auto queue = this->primary_dvc->getQueue(GPGPU_Device_Vulkan::queue_name_graphics);
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
	}

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
	
	std::vector<VkCommandBuffer> presentation_bufs(swp_images.size());
	const VkCommandBufferAllocateInfo allocation_info
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = wnd_ext->cmd_allocations.present,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<uint32_t>(presentation_bufs.size())
	};
	vkAllocateCommandBuffers(commons.dvc, &allocation_info, presentation_bufs.data());

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

	for (auto i = 0; i < swp_images.size(); i++)
	{
		VkImageViewCreateInfo info
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
		wnd_ext->frames.presentation->frames[i].cmd = presentation_bufs[i];
		vkCreateFence(commons.dvc, &fence_info, nullptr, &wnd_ext->frames.presentation->frames[i].cmd_fence);
		vkCreateSemaphore(commons.dvc, &semaphore_info, nullptr, &wnd_ext->frames.presentation->frames[i].cmd_semaphore);
	}
}