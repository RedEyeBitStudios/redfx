#include <internals/subsystems/gpgpu/vulkan/devices.hpp>
#include "../proc_stage_resources/stage_ui_colorbox.hpp"

using ClassImpl = nxcraft::intern::subsystems::GPGPU_Device_Vulkan;

void ClassImpl::createProcessor()
{
	this->makeProcessor<GPGPU_ProcessorStageResources_UI_ColorBox>();
}
void ClassImpl::destroyProcessor()
{
	for (auto& stage : std::views::values(this->processor_data))
	{
		static_cast<GPGPU_ProcessorStageResources_Vulkan*>(stage.get())->flush(this->getCommons());
	}
}