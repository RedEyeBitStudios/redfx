#pragma once
#include <internals/subsystems/gpgpu/vulkan/processor_stage_resources.hpp>
#include <vulkan/vulkan.h>
#include <nx-utils/math/vec4.hpp>

namespace nxcraft::intern::subsystems
{
	using namespace nexora_utils::math;

	class GPGPU_ProcessorStageResources_UI_ColorBox : public GPGPU_ProcessorStageResources_Vulkan
	{
	public:
		const std::vector<ShaderInfo> shaders
		{
			ShaderInfo
			{
				.stage_flags = VK_SHADER_STAGE_VERTEX_BIT,
				.file_name = "ui_colorbox.vert.spv"
			},
			ShaderInfo
			{
				.stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.file_name = "ui_colorbox.frag.spv"
			}
		};

		struct
		{
			VkPipeline handle;
			VkPipelineLayout layout;
			VkDescriptorSetLayout descriptors_layout;

			VkRenderPass pass;
		} render_pipeline;

		struct
		{
			VkDescriptorPool allocation;
			VkDescriptorSet set;
			VkDescriptorUpdateTemplate desc_update_template;
		} descriptors;

		GPGPU_ProcessorStageResources_UI_ColorBox(const GPGPU_Device_Vulkan::Commons& commons);
		virtual ~GPGPU_ProcessorStageResources_UI_ColorBox() = default;
		void flush(const GPGPU_Device_Vulkan::Commons& commons) override;

		struct alignas(8) ColorBox
		{
			ui16vec2 lo_v;
			ui16vec2 hi_v;
			f16vec4 color_rgba;
			uint16_t stack_position;
		};

		static_assert(has_alignment8<ColorBox>());

		struct ShaderResources
		{
			
			/*
			struct TextBoxCharacter : public BoxUI_Base
			{
				f16vec4 color_rgba;
				ui8vec2 character_address;
			};
			struct BitmapBox : public BoxUI_Base
			{
				f16vec4 color_rgba;
				uint8_t bitmap_address;
			};

			*/
		};		
	};
};