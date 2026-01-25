#pragma once
#include <vulkan/vulkan.h>
#include "../../../bases/gpgpu/root_base.hpp"
#include "devices.hpp"

namespace nxcraft::intern::subsystems
{
	class GPGPU_ProcessorStageResources_Vulkan : public GPGPU_Device::GPGPU_ProcessorStageResources
	{
	public:
		struct ShaderInfo
		{
			VkShaderStageFlagBits stage_flags;
			std::string_view file_name;
		};

		GPGPU_ProcessorStageResources_Vulkan() = default;
		virtual ~GPGPU_ProcessorStageResources_Vulkan() = default;
		virtual void flush(const GPGPU_Device_Vulkan::Commons& commons) = 0;
	};
}