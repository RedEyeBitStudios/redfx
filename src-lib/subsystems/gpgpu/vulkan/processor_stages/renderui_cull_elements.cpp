#include "renderui_cull_elements.hpp"
#include <cassert>
#include <ranges>
#include <type_traits>
#include <subsystems.hpp>
#include <locale>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_ProcessorStage_RenderUI_CullElements;

void ClassImpl::process(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, std::any any_data)
{
	auto& cull_result = std::any_cast<std::reference_wrapper<GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp>>(any_data).get();
	for (auto& pg : info.ui_ext->active)
	{
		this->cullColorBoxes(cull_result, *pg);
		this->cullTextBoxes(cull_result, *pg, commons, info);
	}	
}
void ClassImpl::cullColorBoxes(GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp& resources, UI& pg)
{
	for (auto& box : std::views::values(pg.boxes))
	{
		if (box.mask & UI::BoxProperties::BoxMask::IS_ACTIVE)
		{
			const auto backward_rgba = *reinterpret_cast<const ui8vec4*>(&box.color_rgba);
			const auto layer_id = pg.base_layer_id + box.layer_id;

			resources.layers_data[layer_id].color_boxes.push_back
			(
				std::decay_t<decltype(resources)>::UniformData_ColorBox
				{
					.lo_v = box.position_px,
					.hi_v = box.position_px + box.wh_px,
					.color_rgba = f16vec4(backward_rgba.w, backward_rgba.z, backward_rgba.y, backward_rgba.x),
				}
			);
		}
	}
}
void ClassImpl::cullTextBoxes(GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp& resources, UI& pg, GPGPU_Device_Vulkan* commons, VidRoot::VidWindows::VidWndInfo& info)
{
	std::codecvt_utf8<char32_t> codec;
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
	std::unordered_map<uint32_t, std::vector<std::decay_t<decltype(resources)>::UniformData_TextBox>> characters;

	for (auto& box : std::views::values(pg.text_boxes))
	{
		if (box.mask & UI::BoxProperties::BoxMask::IS_ACTIVE)
		{
			const auto backward_rgba = *reinterpret_cast<const ui8vec4*>(&box.color_rgba);
			const auto layer_id = pg.base_layer_id + box.layer_id;
			const auto aspect = static_cast<std::float16_t>(info.mode.wh.y) / static_cast<std::float16_t>(info.mode.wh.x);

			if (commons->retrieveResource(box.resource) == nullptr)
			{
				commons->addTransferQueue(box.resource);
			}
			else
			{
				auto font_view = static_cast<const Subsystems::ResourcesManagerRoot::ResourceData_Font*>(Subsystems::getSubsystem_ResourcesManager().retrieveResourceView(box.resource)->data.get());

				if (font_view == nullptr)
				{
					continue;
				}
				else
				{
					auto pos = box.position_px;
					for (const auto utf32_character : converter.from_bytes(box.text.data()))
					{
						const uint32_t utf32_code = static_cast<uint32_t>(utf32_character);
						auto character = (*font_view)[utf32_code];
						std::float16_t next = 0.5f16;
						if (character != nullptr)
						{
							next = character->width;
							
							characters[utf32_code].push_back
							(
								std::decay_t<decltype(resources)>::UniformData_TextBox
								{
									.color_rgba = f16vec4(backward_rgba.w, backward_rgba.z, backward_rgba.y, backward_rgba.x),
									.offset = static_cast<f16vec2>(pos),
									.size_px = static_cast<std::float16_t>(box.size)
								}
							);
							resources.layers_data[layer_id].text_boxes_info[utf32_code]++;
							/*
							resources.layers_data[layer_id].text_boxes.push_back
							(
								std::decay_t<decltype(resources)>::UniformData_TextBox
								{
									.color_rgba = f16vec4(backward_rgba.w, backward_rgba.z, backward_rgba.y, backward_rgba.x),
									.offset = box.position_px
								}
							);
							*/
						}
						pos.x += box.size * next * aspect;
					}
					for (auto& value : std::views::values(characters))
					{
						resources.layers_data[layer_id].text_boxes.append_range(std::move(value));
					}
				}
			}
		}
	}
}