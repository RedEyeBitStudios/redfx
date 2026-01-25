#pragma once
#include "../../../bases/gpgpu/framedata.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace nxcraft::intern::subsystems
{
	class GPGPU_FrameData_Vulkan : public GPGPU_FrameData
	{
	public:
		VkCommandBuffer cmd = VK_NULL_HANDLE;
	};

	template<typename T>
	class FramesDataUnit_Vulkan final
	{
	public:
		~FramesDataUnit_Vulkan() = default;

		std::vector<T> frames;
		uint32_t current_frame_id = 0;

		FramesDataUnit_Vulkan(const uint32_t size)
		{
			this->frames.resize(size);
		}

		uint32_t increment()
		{
			return (this->current_frame_id + 1) % this->frames.size();
		}
		uint32_t decrement()
		{
			return (this->current_frame_id - 1) % this->frames.size();
		}
	};

	class GPGPU_FrameData_Vulkan_Present : public GPGPU_FrameData_Vulkan
	{
	public:
		VkImage presentation_buffer = VK_NULL_HANDLE;
		VkImageView presentation_buf_view = VK_NULL_HANDLE;

		VkFence cmd_fence = VK_NULL_HANDLE;
		VkSemaphore cmd_semaphore = VK_NULL_HANDLE;
	};

	class GPGPU_FrameData_Vulkan_UI : public GPGPU_FrameData_Vulkan
	{
	public:
		VkImage color_framebuffer = VK_NULL_HANDLE;
		VkImageView color_framebuffer_view = VK_NULL_HANDLE;
		VkBuffer color_box_uniform_buffer = VK_NULL_HANDLE;
		VkFence cmd_fence = VK_NULL_HANDLE;

		VkDescriptorSet desc_set;
	};

	class GPGPU_FrameData_Vulkan_GFX : public GPGPU_FrameData_Vulkan
	{
	public:
		VkImage framebuffer = VK_NULL_HANDLE;
		VkImageView framebuffer_view = VK_NULL_HANDLE;
	};
}