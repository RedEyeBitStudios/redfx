#include <internals/bases/err.hpp>
#include <format>

using ClassImpl = nxcraft::err::GPGPU_Vulkan_LogicalDeviceCreationFailed;

ClassImpl::GPGPU_Vulkan_LogicalDeviceCreationFailed(const int result) : ErrorBase(std::format("Logical device creation failed; error code: {}", static_cast<int>(result)).c_str())
{

}