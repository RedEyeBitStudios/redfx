#pragma once
#include "processor_stage_resources.hpp"
#include "window_extension.hpp"
#include <internals/subsystems/gpgpu/vulkan/devices.hpp>

namespace nxcraft::intern::subsystems
{	
	class GPGPU_ProcessorStage_Vulkan
	{
	public:
		GPGPU_ProcessorStage_Vulkan() = default;
		virtual ~GPGPU_ProcessorStage_Vulkan() = default;
		virtual void process(VidRoot::VidWindows::VidWndInfo& info, GPGPU_ProcessorStageResources_Vulkan* resources, GPGPU_Device_Vulkan* commons) = 0;
	};
}