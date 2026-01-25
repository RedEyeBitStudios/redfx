#pragma once
#include <internals/subsystems/gpgpu/vulkan/processor_stage_resources.hpp>
#include <vulkan/vulkan.h>

namespace nxcraft::intern::subsystems
{
	class GPGPU_ProcessorStageResources_UI_ColorBox : public GPGPU_ProcessorStageResources_Vulkan
	{
	public:
		const ShaderInfo colorbox_shader
		{
			.stage_flags = VK_SHADER_STAGE_COMPUTE_BIT,
			.file_name = "ui.comp.spv"
		};
		VkPipeline pipeline_comp;
		VkPipelineLayout pipeline_layout_comp;
		VkDescriptorSetLayout desc_layout;
		VkDescriptorSet desc_set;
		VkDescriptorPool descs_pool;

		VkDescriptorUpdateTemplate desc_update_template;

		GPGPU_ProcessorStageResources_UI_ColorBox(const GPGPU_Device_Vulkan::Commons& commons);
		virtual ~GPGPU_ProcessorStageResources_UI_ColorBox() = default;
		void flush(const GPGPU_Device_Vulkan::Commons& commons) override;
		
	};
};