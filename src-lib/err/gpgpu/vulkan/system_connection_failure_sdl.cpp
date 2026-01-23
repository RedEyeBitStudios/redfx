#include <internals/bases/err.hpp>
#include <format>

using ClassImpl = nxcraft::err::GPGPU_Vulkan_SystemConnectionFailureSDL;

ClassImpl::GPGPU_Vulkan_SystemConnectionFailureSDL(std::string_view str) : ErrorBase(std::format("Instance creation failed; SDL error : '{}'", str))
{

}