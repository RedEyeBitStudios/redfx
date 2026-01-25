#include <internals/subsystems/gpgpu/vulkan/devices.hpp>
#include <subsystems.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_Device_Vulkan;

void ClassImpl::constructMemManager()
{
	nxcraft::Subsystems::getSubsystem_Logger().registerHeader(&this->memory_manager_data, "SubsystemGPGPU::MemoryManager");
}
void ClassImpl::destroyMemManager()
{
	for (auto mem : std::views::keys(this->memory_manager_data.allocated_blocks))
	{
		this->requestDeallocation(mem);
	}
}