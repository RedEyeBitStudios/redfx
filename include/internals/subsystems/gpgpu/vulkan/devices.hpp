#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <stdfloat>
#include <unordered_map>
#include "../../../bases/gpgpu/root_base.hpp"
#include <concepts>

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
		};
	protected:
		DeviceInfo dvc_info;
		DriverUUID driver_cache_uuid;
		VkPhysicalDevice ph_dvc;
		VkDevice dvc;

		std::unordered_map<std::string_view, std::unique_ptr<GPGPU_ProcessorStageResources>> processor_data;

		template<typename T>
		void makeProcessor() requires(std::is_base_of_v<GPGPU_ProcessorStageResources, T>)
		{
			this->processor_data[typeid(T).name()] = std::move(std::make_unique<T>(this->getCommons()));
		}

		std::unordered_map<std::string_view, QueuePair> queues;
		struct
		{
			std::unordered_map<VkDeviceMemory, MemManagerClasses::MemBlockInfo> allocated_blocks;
		} memory_manager_data;

		void constructMemManager();
		void destroyMemManager();

		void createProcessor();
		void destroyProcessor();

		struct
		{
			VkCommandPool cmd_allocation = VK_NULL_HANDLE;
			VkCommandBuffer cmd_transfer;
		} asyncs;
	public:
		constexpr static const char* queue_name_graphics = "graphics";
		constexpr static const char* queue_name_async_transfer = "async_background";

		GPGPU_Device_Vulkan(const VkPhysicalDevice ph_dvc, nxcraft::err::ErrorHolder& err);
		GPGPU_Device_Vulkan(GPGPU_Device_Vulkan&) = delete;
		GPGPU_Device_Vulkan(GPGPU_Device_Vulkan&&) = delete;
		virtual ~GPGPU_Device_Vulkan();

		GPGPU_Device::DeviceInfo retrieveInfo() override;
		const QueuePair getQueue(std::string_view name);
		const SurfaceCapabilities getSwapchainCapabilities(VkSurfaceKHR surf) const;
		const Commons getCommons() const;

		VkDeviceMemory allocate(const std::vector<VkImage*>& imgs, const std::vector<VkBuffer*>& bufs, VkMemoryPropertyFlags mem_flags);
		VkDeviceMemory allocate_bda(const std::vector<VkBuffer*>& bufs, VkMemoryPropertyFlags mem_flags);
		void requestDeallocation(const VkDeviceMemory m);
		const MemManagerClasses::MemBlockInfo getMemBlockInfo(const VkDeviceMemory m);

		void addTransferQueue(); // TODO: Add implementation of asynchronous transfer queue.
		void submitTransfer();

		template<typename T>
		T* getProcessorStageData()
		{
			return reinterpret_cast<T*>(this->processor_data[typeid(T).name()].get());
		}
	};
}