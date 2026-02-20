#pragma once
#include "processor_stage_resources.hpp"
#include "window_extension.hpp"
#include <internals/subsystems/gpgpu/vulkan/devices.hpp>
#include <any>

namespace nxcraft::intern::subsystems
{	
	class GPGPU_ProcessorStage_Vulkan
	{
	public:
		GPGPU_ProcessorStage_Vulkan() = default;
		virtual ~GPGPU_ProcessorStage_Vulkan() = default;
		virtual void process(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, std::any any_data) = 0;
	};
}