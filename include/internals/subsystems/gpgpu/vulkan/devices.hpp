#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <stdfloat>
#include <unordered_map>
#include "../../../bases/gpgpu/root_base.hpp"

namespace nxcraft::intern::subsystems
{
	class GPGPU_WindowExtension_Vulkan;

	class GPGPU_Device_Vulkan : public GPGPU_Device
	{
	public:
		using DriverUUID = std::array<uint64_t, 2>;

		struct QueuePair
		{
			VkQueue handle = VK_NULL_HANDLE;
			uint32_t queue_family_index = 0;
		};

		struct Commons
		{
			VkPhysicalDevice ph_dvc;
			VkDevice dvc;
			DriverUUID driver_cache_uuid;
		};

		struct SurfaceCapabilities
		{
			VkSurfaceFormatKHR format;
			VkSurfaceCapabilitiesKHR capabilities;
		};

		struct MemManagerClasses
		{
			using Resource = std::variant
			<
				VkImage,
				VkBuffer
			>;
			struct MemBlockInfo
			{
				uint64_t size_summary;
				VkMemoryPropertyFlags mem_flags;
				struct ResourceInfo
				{
					uint64_t bytes_offset;
					Resource res;
				};
				std::vector<ResourceInfo> resources;
			};
			using MemBlock = VkDeviceMemory;	
		};
	protected:
		DeviceInfo dvc_info;
		DriverUUID driver_cache_uuid;
		VkPhysicalDevice ph_dvc;
		VkDevice dvc;

		std::unordered_map<std::string_view, QueuePair> queues;
		struct
		{
			std::unordered_map<VkDeviceMemory, MemManagerClasses::MemBlockInfo> allocated_blocks;
		} memory_manager_data;

		void constructMemManager();
		void destroyMemManager();

		struct
		{
			VkCommandPool cmd_allocation = VK_NULL_HANDLE;
			VkCommandBuffer cmd_transfer;
		} asyncs;
	public:
		constexpr static const char* queue_name_graphics = "graphics";
		constexpr static const char* queue_name_async = "async_background";

		GPGPU_Device_Vulkan(const VkPhysicalDevice ph_dvc, nxcraft::err::ErrorHolder& err);
		GPGPU_Device_Vulkan(GPGPU_Device_Vulkan&) = delete;
		GPGPU_Device_Vulkan(GPGPU_Device_Vulkan&&) = delete;
		virtual ~GPGPU_Device_Vulkan();

		GPGPU_Device::DeviceInfo retrieveInfo() override;
		const QueuePair getQueue(std::string_view name);
		const SurfaceCapabilities getSwapchainCapabilities(VkSurfaceKHR surf) const;
		const Commons getCommons() const;

		MemManagerClasses::MemBlock allocate(const std::vector<VkImage*>& imgs, const std::vector<VkBuffer*>& bufs, VkMemoryPropertyFlags mem_flags);
		void requestDeallocation(const MemManagerClasses::MemBlock m);
		const MemManagerClasses::MemBlockInfo getMemBlockInfo(const MemManagerClasses::MemBlock m);

		void addTransferQueue(); // TODO: Add implementation of asynchronous transfer queue.
		void submitTransfer();
	};
}