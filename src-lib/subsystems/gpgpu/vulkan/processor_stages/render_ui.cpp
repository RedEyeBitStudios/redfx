#include "render_ui.hpp"
#include "../proc_stage_resources/renderui_colorbox.hpp"
#include "../proc_stage_resources/renderui_imagebox.hpp"
#include "../proc_stage_resources/renderui_bitmapbox.hpp"
#include <internals/subsystems/gpgpu/vulkan/functions.hpp>
#include <ranges>

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
	commons->getQueue(&q_index);

	// Merge data.
	using UniformDataTypes = std::decay_t<decltype(data_to_update)>;
	UniformDataTypes::UniformData_ColorBoxes color_boxes;
	UniformDataTypes::UniformData_TextBoxes text_boxes;
	UniformDataTypes::UniformData_ImageBoxes image_boxes;
	UniformDataTypes::UniformData_BitmapBoxes bitmap_boxes;
	
	for (auto& layer : std::views::values(data_to_update.layers_data))
	{
		color_boxes.append_range(layer.color_boxes);
		text_boxes.append_range(layer.text_boxes);
		image_boxes.append_range(layer.image_boxes);
		bitmap_boxes.append_range(layer.bitmap_boxes);
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
		},
		VkBufferMemoryBarrier
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_NONE,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.srcQueueFamilyIndex = q_index,
			.dstQueueFamilyIndex = q_index,
			.buffer = frame->box_image.uniform_buffer,
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
			.buffer = frame->box_bitmap.uniform_buffer,
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
	if (!image_boxes.empty())
	{
		vkCmdUpdateBuffer(frame->cmd, frame->box_image.uniform_buffer, 0, std::span(image_boxes).size_bytes(), image_boxes.data());
	}
	if (!bitmap_boxes.empty())
	{
		vkCmdUpdateBuffer(frame->cmd, frame->box_bitmap.uniform_buffer, 0, std::span(bitmap_boxes).size_bytes(), bitmap_boxes.data());
	}

	for (auto& buf_barrier_info : buffer_barriers)
	{
		buf_barrier_info.srcAccessMask = buf_barrier_info.dstAccessMask;
		buf_barrier_info.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	
	vkCmdPipelineBarrier(frame->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, static_cast<uint32_t>(buffer_barriers.size()), buffer_barriers.data(), static_cast<uint32_t>(image_barriers.size()), image_barriers.data());

	std::vector<VkImageMemoryBarrier> barriers;
	
	for (auto& layer : std::views::values(data_to_update.layers_data))
	{
		const std::vector<std::vector<std::string>*> img_sets
		{
			&layer.images_to_bind,
			&layer.bitmaps_to_bind
		};

		for (auto& img_set : std::span(img_sets))
		{
			for (auto& img : std::span(*img_set))
			{
				auto& resource = std::get<GPGPU_Device_Vulkan::ResourceClasses::Resource_Image>(*commons->retrieveResource(img));
				if (resource.transitioned)
				{
					continue;
				}
				barriers.push_back
				(
					VkImageMemoryBarrier
					{
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
						.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						.srcQueueFamilyIndex = q_index,
						.dstQueueFamilyIndex = q_index,
						.image = resource.image,
						.subresourceRange
						{
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1
						}
					}
				);
				resource.transitioned = true;
			}
		}
	}
	vkCmdPipelineBarrier(frame->cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barriers.size()), barriers.data());
}
void ClassImpl::renderLayer(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::UniformData&& layer)
{
	this->renderLayer_ColorBoxes(info, commons, std::move(layer.color_boxes));
	this->renderLayer_TextBoxes(info, commons, std::move(layer.text_boxes), std::move(layer.text_boxes_info));
	this->renderLayer_ImageBoxes(info, commons, std::move(layer.image_boxes), std::move(layer.images_to_bind));
	this->renderLayer_BitmapBoxes(info, commons, std::move(layer.bitmap_boxes), std::move(layer.bitmaps_to_bind));
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
	std::vector<VkDeviceSize> buffers_offsets(1, 0);

	struct
	{
		f16vec2 multiplier;
		VkDeviceAddress buffer_address;
	} push_constant_data
	{
		.multiplier = f16vec2(1.0f16) / static_cast<f16vec2>(info.mode.wh),
		.buffer_address = frame->box_text.uniform_buffer_address
	};
	vkCmdPushConstants(frame->cmd, commons->getProcessorStageData<GPGPU_ProcessorStageResources_RenderUI_TextBox>()->render_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constant_data), &push_constant_data);
	vkCmdBindPipeline(frame->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, commons->getProcessorStageData<GPGPU_ProcessorStageResources_RenderUI_TextBox>()->render_pipeline.handle);
	for (auto& c : layer_info)
	{
		// TODO: Font must be not constant.
		auto& character_data = std::get<GPGPU_Device_Vulkan::ResourceClasses::Resource_RedFX_Font>(*commons->retrieveResource(c.resource)).characters_data[c.character];
		vkCmdBindVertexBuffers(frame->cmd, 0, 1, &character_data.v_buffer, buffers_offsets.data());
		vkCmdDraw(frame->cmd, character_data.v_count, 1, 0, this->lastTextBoxIndex);
		this->lastTextBoxIndex++;
	}
}
void ClassImpl::renderLayer_ImageBoxes(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::UniformData_ImageBoxes&& layer, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::Info_ImageBoxes&& layer_info)
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
		.buffer_address = frame->box_image.uniform_buffer_address
	};

	auto imagebox_processor_data = commons->getProcessorStageData<GPGPU_ProcessorStageResources_RenderUI_ImageBox>();

	std::vector<VkDescriptorImageInfo> descriptor_images_info(64);

	if (!layer_info.empty())
	{
		for (auto i = 0; i < descriptor_images_info.size(); i++)
		{
			std::string resource_name;
			if (i >= layer_info.size())
			{
				resource_name = layer_info.back();
			}
			else
			{
				resource_name = layer_info[i];
			}
			descriptor_images_info[i] = VkDescriptorImageInfo
			{
				.sampler = imagebox_processor_data->resources.sampler,
				.imageView = std::get<GPGPU_Device_Vulkan::ResourceClasses::Resource_Image>(*commons->retrieveResource(resource_name)).view,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
		}
	}
	const std::vector<VkWriteDescriptorSet> descriptor_writes
	{
		VkWriteDescriptorSet
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = imagebox_processor_data->descriptors.set,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = static_cast<uint32_t>(descriptor_images_info.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = descriptor_images_info.data(),
			.pBufferInfo = nullptr,
			.pTexelBufferView = nullptr
		}
	};

	auto pipeline_layout = imagebox_processor_data->render_pipeline.layout;
	if (!layer_info.empty())
	{
		vkUpdateDescriptorSets(commons->getCommons().dvc, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
		vkCmdPushConstants(frame->cmd, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constant_data), &push_constant_data);
		vkCmdBindPipeline(frame->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, imagebox_processor_data->render_pipeline.handle);
		vkCmdBindDescriptorSets(frame->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &imagebox_processor_data->descriptors.set, 0, nullptr);
		vkCmdDraw(frame->cmd, 4, instances_count, 0, this->lastImageBoxIndex);
	}
	this->lastImageBoxIndex += instances_count;
}
void ClassImpl::renderLayer_BitmapBoxes(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::UniformData_BitmapBoxes&& layer, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::Info_BitmapBoxes&& layer_info)
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
		.buffer_address = frame->box_bitmap.uniform_buffer_address
	};

	auto imagebox_processor_data = commons->getProcessorStageData<GPGPU_ProcessorStageResources_RenderUI_BitmapBox>();

	std::vector<VkDescriptorImageInfo> descriptor_images_info(64);

	if (!layer_info.empty())
	{
		for (auto i = 0; i < descriptor_images_info.size(); i++)
		{
			std::string resource_name;
			if (i >= layer_info.size())
			{
				resource_name = layer_info.back();
			}
			else
			{
				resource_name = layer_info[i];
			}
			descriptor_images_info[i] = VkDescriptorImageInfo
			{
				.sampler = imagebox_processor_data->resources.sampler,
				.imageView = std::get<GPGPU_Device_Vulkan::ResourceClasses::Resource_Image>(*commons->retrieveResource(resource_name)).view,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
		}
	}
	const std::vector<VkWriteDescriptorSet> descriptor_writes
	{
		VkWriteDescriptorSet
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = imagebox_processor_data->descriptors.set,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = static_cast<uint32_t>(descriptor_images_info.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = descriptor_images_info.data(),
			.pBufferInfo = nullptr,
			.pTexelBufferView = nullptr
		}
	};

	auto pipeline_layout = imagebox_processor_data->render_pipeline.layout;
	if (!layer_info.empty())
	{
		vkUpdateDescriptorSets(commons->getCommons().dvc, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
		vkCmdPushConstants(frame->cmd, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_constant_data), &push_constant_data);
		vkCmdBindPipeline(frame->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, imagebox_processor_data->render_pipeline.handle);
		vkCmdBindDescriptorSets(frame->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &imagebox_processor_data->descriptors.set, 0, nullptr);
		vkCmdDraw(frame->cmd, 4, instances_count, 0, this->lastBitmapBoxIndex);
	}
	this->lastBitmapBoxIndex += instances_count;
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