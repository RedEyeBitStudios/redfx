#pragma once
#include "renderui_general.hpp"
#include <vulkan/vulkan.h>
#include <nx-utils/math/vec4.hpp>

namespace nxcraft::intern::subsystems
{
	using namespace nexora_utils::math;

	class GPGPU_ProcessorStageResources_RenderUI_TextBox : public GPGPU_ProcessorStageResources_Vulkan
	{
	protected:
		void preparePipelineLayouts(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data) override;
		void preparePipelines(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data) override;
	public:
		const std::vector<ShaderInfo> shaders
		{
			ShaderInfo
			{
				.stage_flags = VK_SHADER_STAGE_VERTEX_BIT,
				.file_name = "ui_textbox.vert.spv"
			},
			ShaderInfo
			{
				.stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.file_name = "ui_textbox.frag.spv"
			}
		};

		struct
		{
			VkPipeline handle;
			VkPipelineLayout layout;
		} render_pipeline;
		
		GPGPU_ProcessorStageResources_RenderUI_TextBox(const GPGPU_Device_Vulkan::Commons& commons, const GPGPU_ProcessorStageResources_RenderUI_General& renderui_generals);
		virtual ~GPGPU_ProcessorStageResources_RenderUI_TextBox() = default;
		void flush(const GPGPU_Device_Vulkan::Commons& commons) override;
	};
};