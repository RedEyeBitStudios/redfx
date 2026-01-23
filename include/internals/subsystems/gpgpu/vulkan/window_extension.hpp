#pragma once
#include "../../../bases/gpgpu/window_extension.hpp"
#include <vulkan/vulkan.h>
#include "framedata.hpp"
#include "devices.hpp"

namespace nxcraft::intern::subsystems
{
	class GPGPU_WindowExtension_Vulkan : public GPGPU_WindowExtension
	{
	public:
		GPGPU_WindowExtension_Vulkan(GPGPU_WindowExtension_Vulkan&) = delete;
		GPGPU_WindowExtension_Vulkan(GPGPU_WindowExtension_Vulkan&&) = delete;

		GPGPU_WindowExtension_Vulkan() = default;
		virtual ~GPGPU_WindowExtension_Vulkan() = default;

		VkSwapchainKHR swp = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkSemaphore acquire_semaphore = VK_NULL_HANDLE;

		GPGPU_Device_Vulkan::MemManagerClasses::MemBlock memory_gfx;
		GPGPU_Device_Vulkan::MemManagerClasses::MemBlock memory_ui;
		GPGPU_Device_Vulkan::MemManagerClasses::MemBlock memory_present;

		struct
		{
			VkCommandPool present;
			VkCommandPool ui;
			VkCommandPool gfx;
		} cmd_allocations;

		struct
		{
			std::unique_ptr<FramesDataUnit_Vulkan<FrameData_Vulkan_Present>> presentation;
			//FramesDataUnit_Vulkan<FrameData_Vulkan_GFX> gfx;
			//FramesDataUnit_Vulkan<FrameData_Vulkan_UI> ui;
		} frames;
	};
}