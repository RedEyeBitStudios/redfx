#pragma once
#include <vulkan/vulkan.h>

namespace nxcraft::intern::vk
{
	extern PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
	extern PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
	extern PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
	extern PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR;
}