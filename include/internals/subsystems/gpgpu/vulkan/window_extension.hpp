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
		static constexpr const size_t ui_frames_count = 3;

		GPGPU_WindowExtension_Vulkan(GPGPU_WindowExtension_Vulkan&) = delete;
		GPGPU_WindowExtension_Vulkan(GPGPU_WindowExtension_Vulkan&&) = delete;

		GPGPU_WindowExtension_Vulkan() = default;
		virtual ~GPGPU_WindowExtension_Vulkan() = default;

		VkSwapchainKHR swp = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkSemaphore acquire_semaphore = VK_NULL_HANDLE;
		VkSemaphore ui_render_semaphore = VK_NULL_HANDLE;

		VkDeviceMemory memory_gfx;
		
		struct
		{
			struct
			{
				VkDeviceMemory resizable_memory_blocks;
				VkDeviceMemory constant_memory_blocks;
			} ui;
		} allocations;

		struct
		{
			VkCommandPool present;
			VkCommandPool ui;
			VkCommandPool gfx;
		} cmd_allocations;

		struct
		{
			std::unique_ptr<FramesDataUnit_Vulkan<GPGPU_FrameData_Vulkan_Present>> presentation;
			std::unique_ptr<FramesDataUnit_Vulkan<GPGPU_FrameData_Vulkan_UI>> ui;
			//FramesDataUnit_Vulkan<GPGPU_FrameData_Vulkan_GFX> gfx;
		} frames;
	};

	using GPGPU_Processor_Vulkan = void(*)(GPGPU_WindowExtension_Vulkan* ext);
}