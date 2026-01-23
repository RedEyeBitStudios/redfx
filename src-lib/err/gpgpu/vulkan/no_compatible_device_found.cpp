#include <internals/bases/err.hpp>
#include <format>

using ClassImpl = nxcraft::err::GPGPU_Vulkan_NoCompatibleDeviceFound;

ClassImpl::GPGPU_Vulkan_NoCompatibleDeviceFound() : ErrorBase("Do not found any supported device.")
{

}