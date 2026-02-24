#pragma once
#include <internals/subsystems/gpgpu/vulkan/processor.hpp>

#include "../proc_stage_resources/renderui_cull_elements.hpp"
#include "../proc_stage_resources/renderui_textbox.hpp"

namespace nxcraft::intern::subsystems
{
	class GPGPU_ProcessorStage_RenderUI_CullElements : public GPGPU_ProcessorStage_Vulkan
	{
	private:
		void cullColorBoxes(GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp& resources, UI& pg);
		void cullTextBoxes(GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp& resources, UI& pg, GPGPU_Device_Vulkan* commons, VidRoot::VidWindows::VidWndInfo& info);
		void cullImageBoxes(GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp& resources, UI& pg, GPGPU_Device_Vulkan* commons);
	public:
		GPGPU_ProcessorStage_RenderUI_CullElements() = default;
		virtual ~GPGPU_ProcessorStage_RenderUI_CullElements() = default;

		virtual void process(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, std::any any_data) override;
	};
}