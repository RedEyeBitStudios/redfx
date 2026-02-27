#include "renderui_cull_elements.hpp"
#include <cassert>
#include <ranges>
#include <type_traits>
#include <subsystems.hpp>
#include <locale>
#include <algorithm>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_ProcessorStage_RenderUI_CullElements;

void ClassImpl::process(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, std::any any_data)
{
	auto& cull_result = std::any_cast<std::reference_wrapper<GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp>>(std::ref(any_data)).get();
	for (auto& pg : info.ui_ext->active)
	{
		this->cullColorBoxes(cull_result, *pg);
		this->cullTextBoxes(cull_result, *pg, commons, info);
		this->cullImageBoxes(cull_result, *pg, commons);
		this->cullBitmapBoxes(cull_result, *pg, commons);
	}
}
void ClassImpl::cullColorBoxes(GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp& resources, UI& pg)
{
	for (auto& box : std::views::values(pg.boxes))
	{
		if (box.mask & UI::BoxProperties::BoxMask::IS_ACTIVE)
		{
			const auto layer_id = pg.base_layer_id + box.layer_id;

			resources.layers_data[layer_id].color_boxes.push_back
			(
				std::decay_t<decltype(resources)>::UniformData_ColorBox
				{
					.lo_v = box.position_px,
					.hi_v = box.position_px + box.wh_px,
					.color_rgba = this->decodeColorABGR(box.color_rgba),
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
		if (box.mask & UI::BoxProperties::BoxMask::IS_ACTIVE && !box.text.empty())
		{
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
					std::vector<std::decay_t<decltype(resources)>::UniformData_TextBox> boxes_cache;
					boxes_cache.reserve(box.text.length());

					auto pos = box.position_px;
					for (const auto utf32_character : converter.from_bytes(box.text.data()))
					{
						const uint32_t utf32_code = static_cast<uint32_t>(utf32_character);
						auto character = (*font_view)[utf32_code];
						std::float16_t next = 0.5f16;
						if (character != nullptr)
						{
							next = character->width;

							boxes_cache.push_back
							(
								std::decay_t<decltype(resources)>::UniformData_TextBox
								{
									.color_rgba = this->decodeColorABGR(box.color_rgba),
									.offset = static_cast<f16vec2>(pos),
									.size_px = static_cast<std::float16_t>(box.size)
								}
							);
							resources.layers_data[layer_id].text_boxes_info.push_back
							(
								std::decay_t<decltype(resources)>::TextCharacterInfo
								{
									.character = utf32_code,
									.resource = box.resource
								}
							);
						}
						pos.x += box.size * next * aspect;
					}

					if (box.mask & UI::BoxProperties::BoxMask::ALIGNMENT_CENTER)
					{
						const auto pos_difference = boxes_cache.back().offset.x - boxes_cache.front().offset.x;
						const auto dif_to_apply = pos_difference / 2;
						for (auto& entry : boxes_cache)
						{
							entry.offset.x -= dif_to_apply;
						}
					}
					resources.layers_data[layer_id].text_boxes.append_range(std::span(boxes_cache));
				}
			}
		}
	}
}
void ClassImpl::cullImageBoxes(GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp& resources, UI& pg, GPGPU_Device_Vulkan* commons)
{
	for (auto& box : std::views::values(pg.image_boxes))
	{
		const Subsystems::ResourcesManagerRoot::ResourceCache* res = Subsystems::getSubsystem_ResourcesManager().retrieveResourceView(box.resource);
		const auto gpgpu_res = commons->retrieveResource(box.resource);

		if (res->data == nullptr || gpgpu_res == nullptr)
		{
			commons->addTransferQueue(box.resource);
			continue;
		}
		if (box.mask & UI::BoxProperties::BoxMask::IS_ACTIVE)
		{
			const auto layer_id = pg.base_layer_id + box.layer_id;

			auto& images_to_bind = resources.layers_data[layer_id].images_to_bind;

			if (!std::ranges::contains(images_to_bind, box.resource))
			{
				images_to_bind.push_back(box.resource);
			}
			
			const auto resource = static_cast<const Subsystems::ResourcesManagerRoot::ResourceData_Image*>(res->data.get());
			const auto resource_resolution = static_cast<f16vec2>(resource->retrieveResolution());
			const auto aspect = resource_resolution.y / resource_resolution.x;
			const auto height = static_cast<uint16_t>(box.size * aspect);
			
			assert(resource->retrieveFormat() == Subsystems::ResourcesManagerRoot::ResourceData_Image::Format::RGBA8 && "Image box accepts only RGBA format.");			

			const auto iterator = std::ranges::find(images_to_bind, box.resource);
			const uint16_t index = static_cast<uint16_t>(iterator.base() - images_to_bind.data());
			
			resources.layers_data[layer_id].image_boxes.push_back
			(
				std::decay_t<decltype(resources)>::UniformData_ImageBox
				{
					.lo_v = box.position_px,
					.hi_v = box.position_px + ui16vec2(box.size, height),
					.image_index = index
				}
			);
		}
	}
}
void ClassImpl::cullBitmapBoxes(GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp& resources, UI& pg, GPGPU_Device_Vulkan* commons)
{
	for (auto& box : std::views::values(pg.bitmap_boxes))
	{
		const Subsystems::ResourcesManagerRoot::ResourceCache* res = Subsystems::getSubsystem_ResourcesManager().retrieveResourceView(box.resource);
		const auto gpgpu_res = commons->retrieveResource(box.resource);

		if (res->data == nullptr || gpgpu_res == nullptr)
		{
			commons->addTransferQueue(box.resource);
			continue;
		}
		if (box.mask & UI::BoxProperties::BoxMask::IS_ACTIVE)
		{
			const auto layer_id = pg.base_layer_id + box.layer_id;

			auto& images_to_bind = resources.layers_data[layer_id].bitmaps_to_bind;

			if (!std::ranges::contains(images_to_bind, box.resource))
			{
				images_to_bind.push_back(box.resource);
			}
			
			const auto resource = static_cast<const Subsystems::ResourcesManagerRoot::ResourceData_Image*>(res->data.get());
			const auto resource_resolution = static_cast<f16vec2>(resource->retrieveResolution());
			const auto aspect = resource_resolution.y / resource_resolution.x;
			const auto height = static_cast<uint16_t>(box.size * aspect);
			
			assert(resource->retrieveFormat() == Subsystems::ResourcesManagerRoot::ResourceData_Image::Format::BW8 && "Bitmap box accepts only R format.");			

			const auto iterator = std::ranges::find(images_to_bind, box.resource);
			const uint16_t index = static_cast<uint16_t>(iterator.base() - images_to_bind.data());
			
			resources.layers_data[layer_id].bitmap_boxes.push_back
			(
				std::decay_t<decltype(resources)>::UniformData_BitmapBox
				{
					.lo_v = box.position_px,
					.hi_v = box.position_px + ui16vec2(box.size, height),
					.color_rgba = this->decodeColorABGR(box.color_rgba),
					.image_index = index
				}
			);
		}
	}
}
nexora_utils::math::f16vec4 ClassImpl::decodeColorABGR(const uint32_t rgba)
{
	const auto backward_rgba = static_cast<f16vec4>(*reinterpret_cast<const ui8vec4*>(&rgba));
	return f16vec4(backward_rgba.w, backward_rgba.z, backward_rgba.y, backward_rgba.x) / f16vec4(255.0f16);
}