#include <internals/bases/err.hpp>
#include <format>

using ClassImpl = nxcraft::err::GPGPU_Vulkan_SystemConnectionFailure;

ClassImpl::GPGPU_Vulkan_SystemConnectionFailure(const int result) : ErrorBase(std::format("Instance creation failed; error code: {}", static_cast<int>(result)).c_str())
{

}