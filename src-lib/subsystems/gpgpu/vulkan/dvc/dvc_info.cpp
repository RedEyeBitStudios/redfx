#include <internals/subsystems/gpgpu/vulkan/devices.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_Device_Vulkan;

ClassImpl::DeviceInfo ClassImpl::retrieveInfo()
{
	auto& estimated_usage = this->dvc_info.estimated_allocation_usage_bytes;
	estimated_usage = 0;
	for (const auto& mem_region : std::views::values(this->memory_manager_data.allocated_blocks))
	{
		estimated_usage += mem_region.size_summary;
	}

	return this->dvc_info;
}
const ClassImpl::QueuePair ClassImpl::getQueue(std::string_view name)
{
	return this->queues[name];
}
const ClassImpl::MemManagerClasses::MemBlockInfo ClassImpl::getMemBlockInfo(const VkDeviceMemory m)
{
	return this->memory_manager_data.allocated_blocks[m];
}
const ClassImpl::SurfaceCapabilities ClassImpl::getSwapchainCapabilities(VkSurfaceKHR surf) const
{
	return SurfaceCapabilities
	{
		.format = [this, &surf]() -> VkSurfaceFormatKHR
		{
			uint32_t formats_count = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(this->ph_dvc, surf, &formats_count, nullptr);
			std::vector<VkSurfaceFormatKHR> formats(formats_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(this->ph_dvc, surf, &formats_count, formats.data());
			return formats.front();
		}(),
		.capabilities = [this, &surf]() -> VkSurfaceCapabilitiesKHR
		{
			VkSurfaceCapabilitiesKHR capabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->ph_dvc, surf, &capabilities);
			return capabilities;
		}()
	};
}
const ClassImpl::Commons ClassImpl::getCommons() const
{
	return ClassImpl::Commons
	{
		.ph_dvc = this->ph_dvc,
		.dvc = this->dvc,
		.driver_cache_uuid = this->driver_cache_uuid
	};
}