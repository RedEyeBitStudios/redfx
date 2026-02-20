#include <internals/subsystems/gpgpu/vulkan/functions.hpp>

namespace nxcraft::intern::vk
{
	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR = nullptr;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
	PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR = nullptr;
}