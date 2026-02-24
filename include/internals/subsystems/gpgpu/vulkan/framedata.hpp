#pragma once
#include "../../../bases/gpgpu/framedata.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <nx-utils/math/vec2.hpp>

namespace nxcraft::intern::subsystems
{
	class GPGPU_FrameData_Vulkan : public GPGPU_FrameData
	{
	public:
		VkCommandBuffer cmd = VK_NULL_HANDLE;

		enum class FrameState
		{
			FREE,
			RECORDED,
			PENDING
		};
	};

	template<typename T>
	class FramesDataUnit_Vulkan final
	{
	public:
		~FramesDataUnit_Vulkan() = default;

		std::vector<T> frames;
		uint32_t current_frame_id = 0;
		T* latest_frame = nullptr;

		FramesDataUnit_Vulkan(const uint32_t size)
		{
			this->frames.resize(size);
		}

		uint32_t increment()
		{
			return (this->current_frame_id + 1) % this->frames.size();
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
		struct
		{
			struct
			{
				VkImage image = VK_NULL_HANDLE;
				VkImageView view = VK_NULL_HANDLE;
			} resolve;
			struct
			{
				VkImage image = VK_NULL_HANDLE;
				VkImageView view = VK_NULL_HANDLE;
			} msaa;
		} images;

		struct BoxBuffer
		{
			VkBuffer uniform_buffer = VK_NULL_HANDLE;
			VkDeviceAddress uniform_buffer_address;
		};

		BoxBuffer box_color;
		BoxBuffer box_text;
		BoxBuffer box_image;

		VkFence cmd_fence = VK_NULL_HANDLE;
		VkFramebuffer framebuffer;

		nexora_utils::math::ui16vec2 update_bound_lo;
		nexora_utils::math::ui16vec2 update_bound_hi;

		FrameState state = FrameState::FREE;
	};

	class GPGPU_FrameData_Vulkan_GFX : public GPGPU_FrameData_Vulkan
	{
	public:
		VkImage framebuffer = VK_NULL_HANDLE;
		VkImageView framebuffer_view = VK_NULL_HANDLE;
	};
}