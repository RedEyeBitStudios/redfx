#include <internals/subsystems/gpgpu/vulkan/devices.hpp>
#include "../proc_stage_resources/renderui_colorbox.hpp"
#include "../proc_stage_resources/renderui_textbox.hpp"

using ClassImpl = nxcraft::intern::subsystems::GPGPU_Device_Vulkan;

void ClassImpl::createProcessor()
{
	this->makeProcessor<GPGPU_ProcessorStageResources_RenderUI_General>();
	this->makeProcessor<GPGPU_ProcessorStageResources_RenderUI_ColorBox>(*this->getProcessorStageData<GPGPU_ProcessorStageResources_RenderUI_General>());
	this->makeProcessor<GPGPU_ProcessorStageResources_RenderUI_TextBox>(*this->getProcessorStageData<GPGPU_ProcessorStageResources_RenderUI_General>());
}
void ClassImpl::destroyProcessor()
{
	for (auto& stage : std::views::values(this->processor_data))
	{
		static_cast<GPGPU_ProcessorStageResources_Vulkan*>(stage.get())->flush(this->getCommons());
	}
}