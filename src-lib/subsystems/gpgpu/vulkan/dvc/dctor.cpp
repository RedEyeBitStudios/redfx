#include <internals/subsystems/gpgpu/vulkan/devices.hpp>
#include <format>

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

	struct LocalQueueInfo
	{
		std::vector<VkQueueFlags> flags;
		float priority;
		VkQueue* q_ptr;
	};

	std::array<LocalQueueInfo, 2> queues_init_info
	{
		// Graphics queue.
		LocalQueueInfo
		{
			.flags 
			{
				VK_QUEUE_GRAPHICS_BIT,
				VK_QUEUE_COMPUTE_BIT
			},
			.priority = 1.0f,
			.q_ptr = &this->queues[ClassImpl::queue_name_graphics].handle
		},
		// Async queue.
		LocalQueueInfo
		{
			.flags
			{
				VK_QUEUE_TRANSFER_BIT
			},
			.priority = 0.4f,
			.q_ptr = &this->queues[ClassImpl::queue_name_async_transfer].handle
		}
	};
	std::unordered_map<uint32_t, std::vector<LocalQueueInfo*>> queues_info_per_queue_family;

	for (auto& v : queues_init_info)
	{
		uint32_t id = 0;
		for (auto& queue : queues)
		{
			int flags = 0;
			for (auto flag : v.flags)
			{
				if (queue.queueFlags & flag)
				{
					flags++;
				}
			}
			if (flags == v.flags.size())
			{
				id = &queue - queues.data();
			}
		}
		queues_info_per_queue_family[id].push_back(&v);
	}

	std::unordered_map<uint32_t, std::vector<float>> priorities_per_queue_family;
	std::vector<VkDeviceQueueCreateInfo> queue_infos;

	for (auto& queue_info : queues_info_per_queue_family)
	{
		for (auto& q : queue_info.second)
		{
			priorities_per_queue_family[queue_info.first].push_back(q->priority);
		}
	}
	for (auto& priority : priorities_per_queue_family)
	{
		queue_infos.push_back
		(
			VkDeviceQueueCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = priority.first,
				.queueCount = static_cast<uint32_t>(priority.second.size()),
				.pQueuePriorities = reinterpret_cast<const float*>(priority.second.data())
			}
		);
	}
	VkPhysicalDeviceFeatures core_features;
	vkGetPhysicalDeviceFeatures(this->ph_dvc, &core_features);

	const VkPhysicalDeviceVulkan11Features features_core_11
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
		.pNext = nullptr,
		.storageBuffer16BitAccess = true,
		.uniformAndStorageBuffer16BitAccess = true,
		.storagePushConstant16 = true,
		.shaderDrawParameters = true
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
		for (auto& info : queues_info_per_queue_family)
		{
			uint32_t id = 0;
			for (auto& queue : info.second)
			{
				vkGetDeviceQueue(this->dvc, info.first, id, queue->q_ptr);
				id++;
			}
		}

		this->constructMemManager();
		this->createProcessor();
	}
}
ClassImpl::~GPGPU_Device_Vulkan()
{
	vkDeviceWaitIdle(this->dvc);
	this->destroyProcessor();
	this->destroyMemManager();

	vkDestroyDevice(this->dvc, nullptr);
}