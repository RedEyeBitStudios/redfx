#include "render_ui_colorbox.hpp"
#include "../proc_stage_resources/stage_ui_colorbox.hpp"

using ClassImpl = nxcraft::intern::subsystems::GPGPU_ProcessorStage_RenderUI_ColorBox;

void ClassImpl::process(VidRoot::VidWindows::VidWndInfo& info, GPGPU_ProcessorStageResources_Vulkan* resources, GPGPU_Device_Vulkan* device)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu);
	auto frame = &wnd_ext->frames.ui->frames[wnd_ext->frames.ui->increment()];
	auto direct_resources = static_cast<GPGPU_ProcessorStageResources_UI_ColorBox*>(resources);	
	auto queue = device->getQueue(GPGPU_Device_Vulkan::queue_name_graphics);
	auto commons = device->getCommons();

	VkImageMemoryBarrier barrier_info
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = VK_ACCESS_NONE,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.srcQueueFamilyIndex = queue.queue_family_index,
		.dstQueueFamilyIndex = queue.queue_family_index,
		.image = frame->color_framebuffer,
		.subresourceRange
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	vkCmdPipelineBarrier(frame->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_info);

	const VkDescriptorImageInfo desc_info
	{
		.sampler = VK_NULL_HANDLE,
		.imageView = frame->color_framebuffer_view,
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL
	};
	vkUpdateDescriptorSetWithTemplate(commons.dvc, direct_resources->desc_set, direct_resources->desc_update_template, &desc_info);

	const VkImageSubresourceRange ranges
	{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};
	const VkClearColorValue clear_value
	{
		.float32 = { 0.0f, 0.0f, 0.0f, 0.0f }
	};
	
	vkCmdClearColorImage(frame->cmd, frame->color_framebuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1, &ranges);

	barrier_info.oldLayout = barrier_info.newLayout;
	barrier_info.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier_info.srcAccessMask = barrier_info.dstAccessMask;
	barrier_info.dstAccessMask =  VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	vkCmdPipelineBarrier(frame->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_info);

	vkCmdBindPipeline(frame->cmd, VK_PIPELINE_BIND_POINT_COMPUTE, direct_resources->pipeline_comp);
	vkCmdBindDescriptorSets(frame->cmd, VK_PIPELINE_BIND_POINT_COMPUTE, direct_resources->pipeline_layout_comp, 0, 1, &direct_resources->desc_set, 0, nullptr);
	vkCmdDispatch(frame->cmd, 4, 4, 1);

	// TODO: One workgroup renders one square.
	// Each workgroup organizes workload by itself.
}