#include <internals/subsystems/gpgpu/vulkan/devices.hpp>
#include <subsystems.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_Device_Vulkan;

void ClassImpl::requestDeallocation(const VkDeviceMemory m)
{
	if (this->memory_manager_data.allocated_blocks[m].size_summary > 0)
	{
		nxcraft::Subsystems::LogRoot::Message(&this->memory_manager_data, std::format("Requested deallocation of {} B.", this->memory_manager_data.allocated_blocks[m].size_summary));
		vkFreeMemory(this->dvc, m, nullptr);
		this->memory_manager_data.allocated_blocks[m] = {};
	}
}