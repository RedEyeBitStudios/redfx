#pragma once
#include <internals/subsystems/gpgpu/vulkan/processor.hpp>

namespace nxcraft::intern::subsystems
{
	class GPGPU_ProcessorStage_RenderUI_ColorBox : public GPGPU_ProcessorStage_Vulkan
	{
	public:
		GPGPU_ProcessorStage_RenderUI_ColorBox() = default;
		virtual ~GPGPU_ProcessorStage_RenderUI_ColorBox() = default;

		void process(VidRoot::VidWindows::VidWndInfo& info, GPGPU_ProcessorStageResources_Vulkan* resources, GPGPU_Device_Vulkan* commons) override;
	};
}