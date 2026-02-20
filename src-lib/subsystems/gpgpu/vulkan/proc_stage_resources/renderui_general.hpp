#pragma once
#include <internals/subsystems/gpgpu/vulkan/processor_stage_resources.hpp>
#include <vulkan/vulkan.h>
#include <nx-utils/math/vec4.hpp>

namespace nxcraft::intern::subsystems
{
	using namespace nexora_utils::math;

	class GPGPU_ProcessorStageResources_RenderUI_General : public GPGPU_ProcessorStageResources_Vulkan
	{
	public:
		VkRenderPass renderpass;

		GPGPU_ProcessorStageResources_RenderUI_General(const GPGPU_Device_Vulkan::Commons& commons);
		virtual ~GPGPU_ProcessorStageResources_RenderUI_General() = default;
		void flush(const GPGPU_Device_Vulkan::Commons& commons) override;	
	};
};