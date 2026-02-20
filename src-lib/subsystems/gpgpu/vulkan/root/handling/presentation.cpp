#include <internals/subsystems/gpgpu/root_vulkan.hpp>
#include <internals/subsystems/gpgpu/vulkan/window_extension.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_RootVulkan;

void ClassImpl::requestPresentation(VidRoot::VidWindows::VidWndInfo& info)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu);
	auto primary_commons = this->primary_dvc->getCommons();
	uint32_t q_index = 0;
	auto primary_queue = this->primary_dvc->getQueue(&q_index);
	auto ui_curr_frame = wnd_ext->frames.ui->latest_frame;

	vkAcquireNextImageKHR(primary_commons.dvc, wnd_ext->swp, UINT64_MAX, wnd_ext->acquire_semaphore, nullptr, &wnd_ext->frames.presentation->current_frame_id);
	auto curr_frame = &wnd_ext->frames.presentation->frames[wnd_ext->frames.presentation->current_frame_id];
	
	vkResetFences(primary_commons.dvc, 1, &curr_frame->cmd_fence);
	vkResetCommandBuffer(curr_frame->cmd, 0);
	const VkCommandBufferBeginInfo begin_info
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pInheritanceInfo = nullptr
	};
	vkBeginCommandBuffer(curr_frame->cmd, &begin_info);
	VkImageMemoryBarrier swp_barrier
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		.srcQueueFamilyIndex = q_index,
		.dstQueueFamilyIndex = q_index,
		.image = curr_frame->presentation_buffer,
		.subresourceRange
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	vkCmdPipelineBarrier
	(
		curr_frame->cmd,
		VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&swp_barrier
	);
	const VkImageSubresourceRange subresource_range
	{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};
	VkImageBlit region
	{
		.srcSubresource
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
		.srcOffsets
		{
			VkOffset3D
			{
				.x = 0,
				.y = 0,
				.z = 0
			},
			VkOffset3D
			{
				.x = info.mode.wh.x,
				.y = info.mode.wh.y,
				.z = 1
			}
		},
		.dstSubresource
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
		.dstOffsets
		{
			VkOffset3D
			{
				.x = 0,
				.y = 0,
				.z = 0
			},
			VkOffset3D
			{
				.x = info.mode.wh.x,
				.y = info.mode.wh.y,
				.z = 1
			}
		}
	};
	vkCmdBlitImage(curr_frame->cmd, ui_curr_frame->images.resolve.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, curr_frame->presentation_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_NEAREST);
	swp_barrier.oldLayout = swp_barrier.newLayout;
	swp_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	vkCmdPipelineBarrier
	(
		curr_frame->cmd,
		VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&swp_barrier
	);
	vkEndCommandBuffer(curr_frame->cmd);
	const VkCommandBufferSubmitInfo cmd_buf_submit_info
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = nullptr,
		.commandBuffer = curr_frame->cmd,
		.deviceMask = 0
	};
	const std::vector<VkPipelineStageFlags> dst_masks
	{
		VK_PIPELINE_STAGE_TRANSFER_BIT
	};
	const VkSubmitInfo submit_info
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &wnd_ext->acquire_semaphore,
		.pWaitDstStageMask = dst_masks.data(),
		.commandBufferCount = 1,
		.pCommandBuffers = &curr_frame->cmd,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &curr_frame->cmd_semaphore
	};
	vkQueueSubmit(primary_queue, 1, &submit_info, curr_frame->cmd_fence);
	vkWaitForFences(primary_commons.dvc, 1, &curr_frame->cmd_fence, VK_TRUE, UINT64_MAX);
	const VkPresentInfoKHR present_info
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &curr_frame->cmd_semaphore,
		.swapchainCount = 1,
		.pSwapchains = &wnd_ext->swp,
		.pImageIndices = &wnd_ext->frames.presentation->current_frame_id,
		.pResults = nullptr
	};
	vkQueuePresentKHR(primary_queue, &present_info);
}