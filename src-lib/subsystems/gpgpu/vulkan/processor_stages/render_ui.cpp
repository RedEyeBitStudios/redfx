#include "render_ui.hpp"
#include "../proc_stage_resources/renderui_colorbox.hpp"
#include <internals/subsystems/gpgpu/vulkan/functions.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_ProcessorStage_RenderUI;

void ClassImpl::process(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, std::any any_data)
{
	auto& cull_results = std::any_cast<std::reference_wrapper<GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp>>(any_data).get();
	this->updateBuffers(info, commons, cull_results);
	this->startCmd(info, commons);
	for (auto& layer : std::views::values(cull_results.layers_data))
	{
		this->renderLayer(info, commons, std::move(layer));
	}
	this->endCmd(info, commons);
}

void ClassImpl::updateBuffers(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp& data_to_update)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu);
	auto frame = &wnd_ext->frames.ui->frames[wnd_ext->frames.ui->current_frame_id];
	uint32_t q_index = 0;

	// Merge data.
	using UniformDataTypes = std::decay_t<decltype(data_to_update)>;
	UniformDataTypes::UniformData_ColorBoxes color_boxes;
	UniformDataTypes::UniformData_TextBoxes text_boxes;
	
	for (auto& layer : std::views::values(data_to_update.layers_data))
	{
		color_boxes.append_range(layer.color_boxes);
		text_boxes.append_range(layer.text_boxes);
	}

	std::vector<VkBufferMemoryBarrier> buffer_barriers
	{
		VkBufferMemoryBarrier
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_NONE,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.srcQueueFamilyIndex = q_index,
			.dstQueueFamilyIndex = q_index,
			.buffer = frame->box_color.uniform_buffer,
			.offset = 0,
			.size = VK_WHOLE_SIZE
		},
		VkBufferMemoryBarrier
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_NONE,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.srcQueueFamilyIndex = q_index,
			.dstQueueFamilyIndex = q_index,
			.buffer = frame->box_text.uniform_buffer,
			.offset = 0,
			.size = VK_WHOLE_SIZE
		}
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
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = q_index,
			.dstQueueFamilyIndex = q_index,
			.image = frame->images.msaa.image,
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
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
			.srcQueueFamilyIndex = q_index,
			.dstQueueFamilyIndex = q_index,
			.image = frame->images.resolve.image,
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

	vkCmdPipelineBarrier(frame->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, static_cast<uint32_t>(buffer_barriers.size()), buffer_barriers.data(), 0, nullptr);
	
	if (!color_boxes.empty())
	{
		vkCmdUpdateBuffer(frame->cmd, frame->box_color.uniform_buffer, 0, std::span(color_boxes).size_bytes(), color_boxes.data());
	}
	if (!text_boxes.empty())
	{
		vkCmdUpdateBuffer(frame->cmd, frame->box_text.uniform_buffer, 0, std::span(text_boxes).size_bytes(), text_boxes.data());
	}

	for (auto& buf_barrier_info : buffer_barriers)
	{
		buf_barrier_info.srcAccessMask = buf_barrier_info.dstAccessMask;
		buf_barrier_info.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	
	vkCmdPipelineBarrier(frame->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, static_cast<uint32_t>(buffer_barriers.size()), buffer_barriers.data(), static_cast<uint32_t>(image_barriers.size()), image_barriers.data());
}
void ClassImpl::renderLayer(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::UniformData&& layer)
{
	this->renderLayer_ColorBoxes(info, commons, std::move(layer.color_boxes));
	this->renderLayer_TextBoxes(info, commons, std::move(layer.text_boxes), std::move(layer.text_boxes_info));
}

void ClassImpl::renderLayer_ColorBoxes(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::UniformData_ColorBoxes&& layer)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu);
	auto frame = &static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu)->frames.ui->frames[wnd_ext->frames.ui->current_frame_id];

	const auto instances_count = static_cast<uint32_t>(layer.size());

	struct
	{
		f16vec2 multiplier;
		VkDeviceAddress buffer_address;
	} push_constant_data
	{
		.multiplier = f16vec2(1.0f16) / static_cast<f16vec2>(info.mode.wh),
		.buffer_address = frame->box_color.uniform_buffer_address
	};
	vkCmdPushConstants(frame->cmd, commons->getProcessorStageData<GPGPU_ProcessorStageResources_RenderUI_ColorBox>()->render_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constant_data), &push_constant_data);
	vkCmdBindPipeline(frame->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, commons->getProcessorStageData<GPGPU_ProcessorStageResources_RenderUI_ColorBox>()->render_pipeline.handle);
	vkCmdDraw(frame->cmd, 4, instances_count, 0, this->lastColorBoxIndex);
	this->lastColorBoxIndex += instances_count;
}
void ClassImpl::renderLayer_TextBoxes(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::UniformData_TextBoxes&& layer, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::Info_TextBoxes&& layer_info)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu);
	auto frame = &static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu)->frames.ui->frames[wnd_ext->frames.ui->current_frame_id];

	if (layer.empty())
	{
		return;
	}

	//const uint32_t utf32_code = 0x00000041;
	
	std::vector<VkDeviceSize> buffers_offsets(1, 0);

	struct
	{
		f16vec2 multiplier;
		f16vec2 aspect;
		VkDeviceAddress buffer_address;
	} push_constant_data
	{
		.multiplier = f16vec2(1.0f16) / static_cast<f16vec2>(info.mode.wh),
		.aspect = f16vec2(static_cast<std::float16_t>(info.mode.wh.y) / static_cast<std::float16_t>(info.mode.wh.x), 1.0f16),
		.buffer_address = frame->box_text.uniform_buffer_address
	};
	vkCmdPushConstants(frame->cmd, commons->getProcessorStageData<GPGPU_ProcessorStageResources_RenderUI_TextBox>()->render_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constant_data), &push_constant_data);
	vkCmdBindPipeline(frame->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, commons->getProcessorStageData<GPGPU_ProcessorStageResources_RenderUI_TextBox>()->render_pipeline.handle);
	for (auto& c : layer_info)
	{
		auto& character_data = std::get<GPGPU_Device_Vulkan::ResourceClasses::Resource_RedFX_Font>(*commons->retrieveResource("Cutive Mono Regular")).characters_data[c.first];
		vkCmdBindVertexBuffers(frame->cmd, 0, 1, &character_data.v_buffer, buffers_offsets.data());
		vkCmdDraw(frame->cmd, character_data.v_count, c.second, 0, this->lastTextBoxIndex);
		this->lastTextBoxIndex += c.second;
	}
}

void ClassImpl::startCmd(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons)
{
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

	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu);
	auto frame = &static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu)->frames.ui->frames[wnd_ext->frames.ui->current_frame_id];
	
	const VkRenderPassBeginInfo render_pass_info
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = nullptr,
		.renderPass = commons->getProcessorStageData<GPGPU_ProcessorStageResources_RenderUI_General>()->renderpass,
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
	vkCmdBeginRenderPass(frame->cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdSetViewport(frame->cmd, 0, 1, &viewport);
	vkCmdSetScissor(frame->cmd, 0, 1, &scissor);
}
void ClassImpl::endCmd(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu);
	auto frame = &static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu)->frames.ui->frames[wnd_ext->frames.ui->current_frame_id];

	const VkSubpassEndInfo subpass_end_info
	{
		.sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR,
		.pNext = nullptr
	};
	vkCmdEndRenderPass(frame->cmd);
}