#pragma once
#include "renderui_general.hpp"
#include <nx-utils/math/vec4.hpp>

namespace nxcraft::intern::subsystems
{
	using namespace nexora_utils::math;

	class GPGPU_ProcessorStageResources_RenderUI_ImageBox : public GPGPU_ProcessorStageResources_Vulkan
	{
	protected:
		void prepareDescriptors(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data) override;
		void preparePipelineLayouts(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data) override;
		void preparePipelines(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data) override;
	public:
		const std::vector<ShaderInfo> shaders
		{
			ShaderInfo
			{
				.stage_flags = VK_SHADER_STAGE_VERTEX_BIT,
				.file_name = "ui_imagebox.vert.spv"
			},
			ShaderInfo
			{
				.stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.file_name = "ui_imagebox.frag.spv"
			}
		};

		struct
		{
			VkPipeline handle;
			VkPipelineLayout layout;
			VkDescriptorSetLayout descriptors_layout;
		} render_pipeline;

		struct
		{
			VkDescriptorPool allocation;
			VkDescriptorSet set;
			VkDescriptorUpdateTemplate desc_update_template;
		} descriptors;

		struct
		{
			VkSampler sampler;
		} resources;

		GPGPU_ProcessorStageResources_RenderUI_ImageBox(const GPGPU_Device_Vulkan::Commons& commons, const GPGPU_ProcessorStageResources_RenderUI_General& renderui_generals);
		virtual ~GPGPU_ProcessorStageResources_RenderUI_ImageBox() = default;
		void flush(const GPGPU_Device_Vulkan::Commons& commons) override;
	};
};