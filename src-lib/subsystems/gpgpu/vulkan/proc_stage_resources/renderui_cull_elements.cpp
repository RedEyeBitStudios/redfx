#include "renderui_cull_elements.hpp"
#include <cassert>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp;

void ClassImpl::flush(const GPGPU_Device_Vulkan::Commons& commons)
{
	assert(false); // 15-02-2026: This class currently does not support flushing due to no Vulkan resources allocated.
}