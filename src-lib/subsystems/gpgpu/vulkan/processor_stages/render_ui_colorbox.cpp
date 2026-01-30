#include "render_ui_colorbox.hpp"
#include "../proc_stage_resources/stage_ui_colorbox.hpp"
#include <internals/subsystems/gpgpu/vulkan/functions.hpp>
#include <map>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_ProcessorStage_RenderUI_ColorBox;
using ColorBoxList = std::map<uint8_t, nxcraft::intern::subsystems::GPGPU_ProcessorStageResources_UI_ColorBox::ColorBox>;

namespace nxcraft::intern::subsystems
{
	static ColorBoxList makeColorBoxesList(VidRoot::VidWindows::VidWndInfo& info)
	{
		ColorBoxList cache;
		for (auto& page : info.ui_ext->active)
		{
			for (auto& box : page->boxes)
			{
				if (!box.second.is_active) continue;

				const auto color_array = *reinterpret_cast<const ui8vec4*>(&box.second.color_rgba);

				

				cache[box.second.depth] = GPGPU_ProcessorStageResources_UI_ColorBox::ColorBox
				{
					.lo_v = box.second.position_px,
					.hi_v = box.second.position_px + box.second.wh_px,
					.color_rgba =  f16vec4(color_array.w, color_array.z, color_array.y, color_array.x),
					.stack_position = box.second.depth
				};
			}
		}
		return cache;
	}
}

void ClassImpl::process(VidRoot::VidWindows::VidWndInfo& info, GPGPU_ProcessorStageResources_Vulkan* resources, GPGPU_Device_Vulkan* device)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu);
	auto frame = &wnd_ext->frames.ui->frames[wnd_ext->frames.ui->current_frame_id];
	auto stage_resources = static_cast<GPGPU_ProcessorStageResources_UI_ColorBox*>(resources);	
	auto queue = device->getQueue(GPGPU_Device_Vulkan::queue_name_graphics);
	auto commons = device->getCommons();

	VkBufferMemoryBarrier buf_barrier_info
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = VK_ACCESS_NONE,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.srcQueueFamilyIndex = queue.queue_family_index,
		.dstQueueFamilyIndex = queue.queue_family_index,
		.buffer = frame->uniform_buffer,
		.offset = 0,
		.size = VK_WHOLE_SIZE
	};
	const std::vector<VkImageMemoryBarrier> image_barriers
	{
		VkImageMemoryBarrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = queue.queue_family_index,
			.dstQueueFamilyIndex = queue.queue_family_index,
			.image = frame->depth_buffer,
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		},
		VkImageMemoryBarrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
		}
	};

	vkCmdPipelineBarrier(frame->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buf_barrier_info, 0, nullptr);

	const auto colorbox_list = makeColorBoxesList(info);
	std::vector<GPGPU_ProcessorStageResources_UI_ColorBox::ColorBox> boxes{};
	boxes.reserve(colorbox_list.size());
	for (auto& box : colorbox_list)
	{
		boxes.push_back(box.second);
	}
	vkCmdUpdateBuffer(frame->cmd, frame->uniform_buffer, 0, std::span(boxes).size_bytes(), boxes.data());


	buf_barrier_info.srcAccessMask = buf_barrier_info.dstAccessMask;
	buf_barrier_info.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(frame->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 1, &buf_barrier_info, static_cast<uint32_t>(image_barriers.size()), image_barriers.data());

	const std::vector<VkClearValue> clears
	{
		VkClearValue
		{
			.depthStencil = { .depth = 0.0f }
		},
		VkClearValue
		{
			.color = { .float32 = { 0, 0, 0, 0, } }
		}
	};

	const VkViewport viewport
	{
		.x = 0,
		.y = 0,
		.width = static_cast<float>(info.mode.wh.x),
		.height = static_cast<float>(info.mode.wh.y),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	const VkRect2D scissor
	{
		.offset = {},
		.extent
		{
			.width = info.mode.wh.x,
			.height = info.mode.wh.y
		}
	};
	
	const VkRenderPassBeginInfo render_pass_info
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = nullptr,
		.renderPass = stage_resources->render_pipeline.pass,
		.framebuffer = frame->framebuffer,
		.renderArea
		{
			.offset = {},
			.extent
			{
				.width = info.mode.wh.x,
				.height = info.mode.wh.y
			}
		},
		.clearValueCount = static_cast<uint32_t>(clears.size()),
		.pClearValues = clears.data()
	};
	const VkSubpassBeginInfo subpass_info
	{
		.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR,
		.pNext = nullptr,
		.contents = VK_SUBPASS_CONTENTS_INLINE
	};
	nxcraft::intern::vk::vkCmdBeginRenderPass2KHR(frame->cmd, &render_pass_info, &subpass_info);

	vkCmdBindPipeline(frame->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, stage_resources->render_pipeline.handle);

	struct
	{
		f16vec2 multiplier;
		VkDeviceAddress buffer_address;
	} push_constant_data
	{
		.multiplier = f16vec2(1.0f16) / static_cast<f16vec2>(info.mode.wh),
		.buffer_address = frame->uniform_buffer_address
	};

	vkCmdPushConstants(frame->cmd, stage_resources->render_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constant_data), &push_constant_data);
	vkCmdSetViewport(frame->cmd, 0, 1, &viewport);
	vkCmdSetScissor(frame->cmd, 0, 1, &scissor);

	vkCmdDraw(frame->cmd, 4, static_cast<uint32_t>(boxes.size()), 0, 0);

	const VkSubpassEndInfo subpass_end_info
	{
		.sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR,
		.pNext = nullptr
	};
	nxcraft::intern::vk::vkCmdEndRenderPass2KHR(frame->cmd, &subpass_end_info);

	/*
	vkUpdateDescriptorSetWithTemplate(commons.dvc, direct_resources->desc_set, direct_resources->desc_update_template, descs.data());
	vkCmdBindPipeline(frame->cmd, VK_PIPELINE_BIND_POINT_COMPUTE, direct_resources->pipeline_comp);
	vkCmdBindDescriptorSets(frame->cmd, VK_PIPELINE_BIND_POINT_COMPUTE, direct_resources->pipeline_layout_comp, 0, 1, &direct_resources->desc_set, 0, nullptr);
	*/
}