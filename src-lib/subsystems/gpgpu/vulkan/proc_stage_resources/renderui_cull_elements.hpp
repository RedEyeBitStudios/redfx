#pragma once
#include <internals/subsystems/gpgpu/vulkan/processor_stage_resources.hpp>
#include <vulkan/vulkan.h>
#include <nx-utils/math/vec4.hpp>
#include <map>
#include <vector>

namespace nxcraft::intern::subsystems
{
	using namespace nexora_utils::math;

	class GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp
	{
	public:
		struct alignas(8) UniformData_ColorBox
		{
			ui16vec2 lo_v;
			ui16vec2 hi_v;
			f16vec4 color_rgba;
		};
		struct alignas(8) UniformData_TextBox
		{
			f16vec4 color_rgba;
			f16vec2 offset;
			std::float16_t size_px;
		};
		struct alignas(8) UniformData_ImageBox
		{
			ui16vec2 lo_v;
			ui16vec2 hi_v;
			uint16_t image_index;
		};

		using UniformData_ColorBoxes = std::vector<UniformData_ColorBox>;
		using UniformData_TextBoxes = std::vector<UniformData_TextBox>;
		using UniformData_ImageBoxes = std::vector<UniformData_ImageBox>;
		using Info_TextBoxes = std::unordered_map<uint32_t, uint32_t>;
		using Info_ImageBoxes = std::vector<std::string>;
		
		struct UniformData
		{
			UniformData_ColorBoxes color_boxes;
			UniformData_TextBoxes text_boxes;
			Info_TextBoxes text_boxes_info;
			UniformData_ImageBoxes image_boxes;
			Info_ImageBoxes images_to_bind;
		};

		using LayersData = std::map<uint8_t, UniformData>;

		LayersData layers_data;

		GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp() = default;
		virtual ~GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp() = default;
	};
};