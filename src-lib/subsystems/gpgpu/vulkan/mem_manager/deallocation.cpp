#include <internals/subsystems/gpgpu/vulkan/devices.hpp>
#include <subsystems.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_Device_Vulkan;

void ClassImpl::requestDeallocation(const MemManagerClasses::MemBlock m)
{
	const auto mem_block = static_cast<VkDeviceMemory>(m);
	this->memory_manager_data.allocated_blocks.erase(mem_block);

	vkFreeMemory(this->dvc, mem_block, nullptr);
}