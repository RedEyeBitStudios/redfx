#pragma once
#include <vulkan/vulkan.h>
#include "../../../bases/gpgpu/root_base.hpp"
#include "devices.hpp"
#include <any>

namespace nxcraft::intern::subsystems
{
	class GPGPU_ProcessorStageResources_Vulkan : public GPGPU_Device::GPGPU_ProcessorStageResources
	{
	protected:
		virtual void prepareDescriptors(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data);
		virtual void preparePipelines(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data);
		virtual void preparePipelineLayouts(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data);
	public:
		struct ShaderInfo
		{
			VkShaderStageFlagBits stage_flags;
			std::string_view file_name;
		};

		GPGPU_ProcessorStageResources_Vulkan() = default;
		virtual ~GPGPU_ProcessorStageResources_Vulkan() = default;
		virtual void flush(const GPGPU_Device_Vulkan::Commons& commons) = 0;
		static VkShaderModule makeShader(const GPGPU_Device_Vulkan::Commons& commons, std::string_view path);

		template<typename T>
		static consteval size_t has_alignment8()
		{
			return (sizeof(T) % 8 == 0 ? sizeof(T) : 0);
		};
	};
}