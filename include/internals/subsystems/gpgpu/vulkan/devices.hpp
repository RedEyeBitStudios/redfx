#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <stdfloat>
#include <unordered_map>
#include "../../../bases/gpgpu/root_base.hpp"
#include <concepts>
#include <thread>

namespace nxcraft::intern::subsystems
{
	class GPGPU_WindowExtension_Vulkan;

	class GPGPU_Device_Vulkan : public GPGPU_Device
	{
	public:
		using DriverUUID = std::array<uint64_t, 2>;

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

		struct ResourceClasses
		{
			struct Resource_RedFX_Font
			{
				VkDeviceMemory mem_block;

				struct CharacterData
				{
					uint32_t v_count;
					VkBuffer v_buffer;
				};

				std::unordered_map<uint32_t, CharacterData> characters_data;
			};
		};

		using GPGPU_Resource = std::variant
		<
			ResourceClasses::Resource_RedFX_Font
		>;
	protected:
		DeviceInfo dvc_info;
		DriverUUID driver_cache_uuid;
		VkPhysicalDevice ph_dvc;
		VkDevice dvc;

		std::unordered_map<std::string_view, std::unique_ptr<GPGPU_ProcessorStageResources>> processor_data;

		template<typename T, typename... AdditionalArgs>
		void makeProcessor(AdditionalArgs... args) requires(std::is_base_of_v<GPGPU_ProcessorStageResources, T>)
		{
			this->processor_data[typeid(T).name()] = std::move(std::make_unique<T>(this->getCommons(), args...));
		}

		class AsyncUploadUnit
		{
		public:
			VkDeviceMemory buffer_mem;
			VkBuffer staging_buffer;
			VkCommandBuffer cmd;
			GPGPU_Device_Vulkan* dvc = nullptr;
			VkFence fence;
			
			AsyncUploadUnit(GPGPU_Device_Vulkan& dvc);
			virtual ~AsyncUploadUnit();
		
			std::vector<std::string_view> assets_to_transfer;

			void controlUpload();
		};

		struct
		{
			uint32_t queue_family_index = 0;
			VkQueue handle;
		} main_queue;

		struct
		{
			struct
			{
				uint32_t family_index = 0;
				VkQueue handle;
			} queue;
			
			VkCommandPool cmd_allocation = VK_NULL_HANDLE;
			std::vector<std::unique_ptr<AsyncUploadUnit>> transfer_units;
			std::vector<std::string_view> asset_queue;
		} transfer;
		
		struct
		{
			std::unordered_map<VkDeviceMemory, MemManagerClasses::MemBlockInfo> allocated_blocks;
			std::unordered_map<std::string, GPGPU_Resource> assets_gpu;
		} memory_manager_data;

		void constructMemManager();
		void destroyMemManager();

		void createProcessor();
		void destroyProcessor();

		void destroyTransferQueues();
		void waitForTransfer(AsyncUploadUnit* unit);
	public:
		
		GPGPU_Device_Vulkan(const VkPhysicalDevice ph_dvc, nxcraft::err::ErrorHolder& err);
		GPGPU_Device_Vulkan(GPGPU_Device_Vulkan&) = delete;
		GPGPU_Device_Vulkan(GPGPU_Device_Vulkan&&) = delete;
		virtual ~GPGPU_Device_Vulkan();

		GPGPU_Device::DeviceInfo retrieveInfo() override;
		const VkQueue getQueue(uint32_t* family_index = nullptr);
		const SurfaceCapabilities getSwapchainCapabilities(VkSurfaceKHR surf) const;
		const Commons getCommons() const;

		[[nodiscard]] VkDeviceMemory allocate(const std::vector<VkImage*>& imgs, const std::vector<VkBuffer*>& bufs, VkMemoryPropertyFlags mem_flags);
		[[nodiscard]] VkDeviceMemory allocate_bda(const std::vector<VkBuffer*>& bufs, VkMemoryPropertyFlags mem_flags);
		void requestDeallocation(const VkDeviceMemory m);
		const MemManagerClasses::MemBlockInfo getMemBlockInfo(const VkDeviceMemory m);

		void addTransferQueue(std::string_view asset_name); // TODO: Add implementation of asynchronous transfer queue.
		GPGPU_Resource* retrieveResource(std::string_view asset_name);
		void submitTransfer();
		

		template<typename T>
		T* getProcessorStageData()
		{
			return reinterpret_cast<T*>(this->processor_data[typeid(T).name()].get());
		}
	};
}