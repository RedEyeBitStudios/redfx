#pragma once
#include <internals/subsystems/gpgpu/vulkan/processor.hpp>
#include "renderui_cull_elements.hpp"

namespace nxcraft::intern::subsystems
{
	class GPGPU_ProcessorStage_RenderUI : public GPGPU_ProcessorStage_Vulkan
	{
	private:
		uint32_t lastColorBoxIndex = 0;
		uint32_t lastTextBoxIndex = 0;
		uint32_t lastImageBoxIndex = 0;

		void updateBuffers(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp& data_to_update);
		void startCmd(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons);
		void renderLayer(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::UniformData&& layer);
		void renderLayer_ColorBoxes(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::UniformData_ColorBoxes&& layer);
		void renderLayer_TextBoxes(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::UniformData_TextBoxes&& layer, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::Info_TextBoxes&& layer_info);
		void renderLayer_ImageBoxes(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::UniformData_ImageBoxes&& layer, GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp::Info_ImageBoxes&& layer_info);
		
		void endCmd(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons);
	public:
		GPGPU_ProcessorStage_RenderUI() = default;
		virtual ~GPGPU_ProcessorStage_RenderUI() = default;

		void process(VidRoot::VidWindows::VidWndInfo& info, GPGPU_Device_Vulkan* commons, std::any any_data) override;
	};
}