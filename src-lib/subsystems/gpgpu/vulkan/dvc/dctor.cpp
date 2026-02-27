#include <internals/subsystems/gpgpu/vulkan/devices.hpp>
#include <format>
#include <subsystems.hpp>
#include <type_traits>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_Device_Vulkan;

extern const uint32_t vulkan_version;
extern const std::array<const char*, 5> required_dvc_extensions_names;

ClassImpl::GPGPU_Device_Vulkan(const VkPhysicalDevice ph_dvc, nxcraft::err::ErrorHolder& err)
{
	this->ph_dvc = ph_dvc;

	VkPhysicalDeviceProperties2 props
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		.pNext = nullptr
	};
	vkGetPhysicalDeviceProperties2(ph_dvc, &props);

	this->dvc_info = ClassImpl::DeviceInfo
	{
		.name = props.properties.deviceName,
		.driver = std::format
		(
			"{}.{}.{}",
			VK_VERSION_MAJOR(props.properties.driverVersion),
			VK_VERSION_MINOR(props.properties.driverVersion),
			VK_VERSION_PATCH(props.properties.driverVersion)
		)
	};
	std::ranges::copy
	(
		std::span<uint64_t>(reinterpret_cast<uint64_t*>(props.properties.pipelineCacheUUID),
		sizeof(props.properties.pipelineCacheUUID) / sizeof(uint64_t)),
		this->driver_cache_uuid.begin()
	);
	VkPhysicalDeviceMemoryProperties2 dvc_memory
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
		.pNext = nullptr
	};
	vkGetPhysicalDeviceMemoryProperties2(this->ph_dvc, &dvc_memory);

	for (uint32_t i = 0; i < dvc_memory.memoryProperties.memoryTypeCount; i++)
	{
		if (dvc_memory.memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
		{
			this->dvc_info.available_memory_bytes = dvc_memory.memoryProperties.memoryHeaps[i].size;
			break;
		}
	}

	const auto queues = [&ph_dvc]() -> std::vector<VkQueueFamilyProperties>
	{
		uint32_t queues_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(ph_dvc, &queues_count, nullptr);
		std::vector<VkQueueFamilyProperties> queues(queues_count);
		vkGetPhysicalDeviceQueueFamilyProperties(ph_dvc, &queues_count, queues.data());
		return queues;
	}();

	std::optional<uint32_t> additional_transfer_queue_family_index = std::nullopt;

	for (auto& queue : queues)
	{
		const uint32_t index = &queue - queues.data();
		if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT && queue.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			this->main_queue.queue_family_index = index;
		}
		else if (queue.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			additional_transfer_queue_family_index.emplace(index);
		}
	}

	const float main_queue_priorities[] = { 1.0f };

	std::vector<VkDeviceQueueCreateInfo> queue_infos
	{
		VkDeviceQueueCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.queueFamilyIndex = this->main_queue.queue_family_index,
			.queueCount = 1,
			.pQueuePriorities = main_queue_priorities
		}
	};

	const std::vector<float> transfer_priorities(1, 0.3f);
	if (additional_transfer_queue_family_index.has_value())
	{
		queue_infos.push_back
		(
			VkDeviceQueueCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = additional_transfer_queue_family_index.value(),
				.queueCount = static_cast<uint32_t>(transfer_priorities.size()),
				.pQueuePriorities = transfer_priorities.data()
			}
		);
	}

	VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR separate_depth_stencil_features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES_KHR,
		.pNext = nullptr,
		.separateDepthStencilLayouts = true
	};
	VkPhysicalDeviceShaderFloat16Int8FeaturesKHR shader8_features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR,
		.pNext = &separate_depth_stencil_features,
		.shaderFloat16 = true,
		.shaderInt8 = false
	};
	VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR,
		.pNext = &shader8_features,
		.bufferDeviceAddress = true
	};
	const VkPhysicalDeviceFeatures core_features
	{
		.tessellationShader = true,
		.sampleRateShading = true,
		.fillModeNonSolid = true,
		.samplerAnisotropy = true,
		.shaderInt16 = true,
	};
	const VkPhysicalDeviceVulkan11Features features_core_11
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
		.pNext = &bda_features,
		.storageBuffer16BitAccess = true,
		.uniformAndStorageBuffer16BitAccess = true,
		.storagePushConstant16 = true,
		.storageInputOutput16 = true,
		.shaderDrawParameters = true,
	};
	const VkDeviceCreateInfo dvc_creation_info
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features_core_11,
		.flags = 0,
		.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size()),
		.pQueueCreateInfos = queue_infos.data(),
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = static_cast<uint32_t>(required_dvc_extensions_names.size()),
		.ppEnabledExtensionNames = required_dvc_extensions_names.data(),
		.pEnabledFeatures = &core_features
	};

	if (const auto result = vkCreateDevice(this->ph_dvc, &dvc_creation_info, nullptr, &this->dvc); result != VK_SUCCESS)
	{
		err.reset(new nxcraft::err::GPGPU_Vulkan_LogicalDeviceCreationFailed(static_cast<int>(result)));
		return;
	}
	else
	{
		vkGetDeviceQueue(this->dvc, this->main_queue.queue_family_index, 0, &this->main_queue.handle);

		auto& additional_queue = this->transfer.queue;
		if (additional_transfer_queue_family_index.has_value())
		{
			additional_queue.family_index = additional_transfer_queue_family_index.value();
			vkGetDeviceQueue(this->dvc, additional_queue.family_index, 0, &additional_queue.handle);
		}
		else
		{
			additional_queue.family_index = this->main_queue.queue_family_index;
			additional_queue.handle = this->main_queue.handle;
		}

		nxcraft::Subsystems::getSubsystem_Logger().registerHeader(&this->processor_data, "SubsystemGPGPU::ResourceManager");

		this->constructMemManager();
		this->createProcessor();

		nxcraft::Subsystems::getSubsystem_Logger().registerHeader(&this->transfer, "SubsystemGPGPU::ResourceManager");

		const VkCommandPoolCreateInfo cmd_allocation_info
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = additional_queue.family_index
		};
		vkCreateCommandPool(this->dvc, &cmd_allocation_info, nullptr, &this->transfer.cmd_allocation);

		this->transfer.transfer_units.resize(2);

		for (auto& unit : this->transfer.transfer_units)
		{
			unit = std::move(std::make_unique<AsyncUploadUnit>(*this));
		}
	}
}
ClassImpl::~GPGPU_Device_Vulkan()
{
	vkDeviceWaitIdle(this->dvc);
	this->destroyTransferQueues();
	this->destroyProcessor();
	this->destroyMemManager();

	vkDestroyDevice(this->dvc, nullptr);
}
void ClassImpl::destroyTransferQueues()
{
	for (auto& asset : this->memory_manager_data.assets_gpu)
	{
		std::visit
		(
			[this](auto& asset)
			{
				using T = std::decay_t<decltype(asset)>;

				if constexpr (std::is_same_v<T, ClassImpl::ResourceClasses::Resource_RedFX_Font>)
				{
					for (auto& data : std::views::values(asset.characters_data))
					{
						vkDestroyBuffer(this->dvc, data.v_buffer, nullptr);
					}
				}
				else if constexpr (std::is_same_v<T, ClassImpl::ResourceClasses::Resource_Image>)
				{
					vkDestroyImage(this->dvc, asset.image, nullptr);
					vkDestroyImageView(this->dvc, asset.view, nullptr);
				}
				else
				{
					static_assert(false, "Branch unimplemented.");
				}
			},
			asset.second
		);
	}
	this->transfer.transfer_units.clear();
	vkDestroyCommandPool(this->dvc, this->transfer.cmd_allocation, nullptr);
}

ClassImpl::AsyncUploadUnit::AsyncUploadUnit(GPGPU_Device_Vulkan& dvc)
{
	this->dvc = &dvc;

	const VkCommandBufferAllocateInfo cmd_info
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = dvc.transfer.cmd_allocation,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	vkAllocateCommandBuffers(dvc.dvc, &cmd_info, &this->cmd);

	const VkFenceCreateInfo fence_info
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};
	vkCreateFence(dvc.dvc, &fence_info, nullptr, &this->fence);

	const VkBufferCreateInfo buffer_info
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.size = 512 * 1024 * 1024,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr
	};
	vkCreateBuffer(dvc.dvc, &buffer_info, nullptr, &this->staging_buffer);

	this->buffer_mem = dvc.allocate({}, {&this->staging_buffer}, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}
ClassImpl::AsyncUploadUnit::~AsyncUploadUnit()
{
	vkDestroyBuffer(this->dvc->dvc, this->staging_buffer, nullptr);
	vkDestroyFence(this->dvc->dvc, this->fence, nullptr);
}