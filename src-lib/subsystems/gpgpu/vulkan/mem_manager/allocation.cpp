#include <internals/subsystems/gpgpu/vulkan/devices.hpp>
#include <subsystems.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_Device_Vulkan;

static std::optional<uint32_t> queryMemoryTypeIndex(VkMemoryPropertyFlags preferred_memory_type, uint32_t required_memory_types, VkPhysicalDevice ph_dvc)
{
	VkPhysicalDeviceMemoryProperties2 dvc_memory_info
	{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
		.pNext = nullptr
	};
	vkGetPhysicalDeviceMemoryProperties2(ph_dvc, &dvc_memory_info);

	for (uint32_t i = 0; i < dvc_memory_info.memoryProperties.memoryTypeCount; i++)
	{
		if ((required_memory_types & (1ul << i)) && (preferred_memory_type & dvc_memory_info.memoryProperties.memoryTypes[i].propertyFlags))
		{
			return i;
		}
	}

	return std::nullopt;
}
template<typename T>
static VkMemoryRequirements getMemoryRequirements(const T res, VkDevice dvc)
{
	VkMemoryRequirements cache;
	if constexpr (std::is_same_v<T, VkImage>)
	{
		vkGetImageMemoryRequirements(dvc, res, &cache);
	}
	else if constexpr (std::is_same_v<T, VkBuffer>)
	{
		vkGetBufferMemoryRequirements(dvc, res, &cache);
	}
	else
	{
		static_assert(false, "Undefined branch.");
	}

	return cache;
}
static size_t computeRequiredSize(const VkMemoryRequirements& requirements)
{
	const size_t memory_to_align = requirements.size % requirements.alignment;
	const size_t memory_without_align = (requirements.size - memory_to_align) / requirements.alignment;

	const size_t required_segments_count = memory_without_align + (memory_to_align > 0 ? 1 : 0);
	return static_cast<size_t>(requirements.alignment) * required_segments_count;
}

template<typename T>
static ClassImpl::MemManagerClasses::Resource bindMemory(const T* res, const size_t offset, VkDeviceMemory mem, VkDevice dvc)
{
	if constexpr (std::is_same_v<T, VkImage>)
	{
		vkBindImageMemory(dvc, *res, mem, offset);
	}
	else if constexpr (std::is_same_v<T, VkBuffer>)
	{
		vkBindBufferMemory(dvc, *res, mem, offset);
	}

	return ClassImpl::MemManagerClasses::Resource(*res);
}

ClassImpl::MemManagerClasses::MemBlock ClassImpl::allocate(const std::vector<VkImage*>& imgs, const std::vector<VkBuffer*>& bufs, VkMemoryPropertyFlags mem_flags)
{
	size_t size_sum = 0;
	std::vector<MemManagerClasses::MemBlockInfo::ResourceInfo> resources_info;
	std::optional<uint32_t> memory_index = std::nullopt;

	for (auto& img : imgs)
	{
		const auto requirements = getMemoryRequirements(*img, this->dvc);
		const size_t required_size = computeRequiredSize(requirements);

		resources_info.push_back
		(
			MemManagerClasses::MemBlockInfo::ResourceInfo
			{
				.bytes_offset = size_sum,
				.res = MemManagerClasses::Resource(*img)
			}
		);

		size_sum += required_size;

		if (!memory_index.has_value())
		{
			memory_index = queryMemoryTypeIndex(mem_flags, requirements.memoryTypeBits, this->ph_dvc);
		}
	}
	for (auto& buf : bufs)
	{
		const auto requirements = getMemoryRequirements(*buf, this->dvc);
		const size_t required_size = computeRequiredSize(requirements);

		resources_info.push_back
		(
			MemManagerClasses::MemBlockInfo::ResourceInfo
			{
				.bytes_offset = size_sum,
				.res = MemManagerClasses::Resource(*buf)
			}
		);

		size_sum += required_size;

		if (!memory_index.has_value())
		{
			memory_index = queryMemoryTypeIndex(mem_flags, requirements.memoryTypeBits, this->ph_dvc);
		}
	}
	const VkMemoryAllocateInfo allocate_info
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = size_sum,
		.memoryTypeIndex = memory_index.value_or(0)
	};
	nxcraft::Subsystems::LogRoot::Message(this, std::format("Requested allocation of {} B.", size_sum));

	VkDeviceMemory memory_block = VK_NULL_HANDLE;
	if (const auto result = vkAllocateMemory(this->dvc, &allocate_info, nullptr, &memory_block); result != VK_SUCCESS)
	{
		nxcraft::Subsystems::LogRoot::Message(this, std::format("Allocation failed; error code: {}", static_cast<int>(result)), nxcraft::Subsystems::LogRoot::Message::Flags::MARK_AS_CRITICAL_ERROR);
		std::string msg;
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
		{
			msg = std::format("VulkanAllocator: Application can not continue; Not enough video processor memory.", static_cast<int>(result));
			
		}
		else if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
		{
			msg = std::format("VulkanAllocator: Application can not continue; Not enough processor memory.", static_cast<int>(result));
		}
		else
		{
			msg = std::format("VulkanAllocator: Application can not continue; Unknown error.", static_cast<int>(result));
		}

		nxcraft::Subsystems::LogRoot::Message
		(
			this,
			msg, 
			nxcraft::Subsystems::LogRoot::Message::Flags::MARK_AS_CRITICAL_ERROR | 
			nxcraft::Subsystems::LogRoot::Message::Flags::SHOW_MESSAGE_BOX
		);
	}

	for (auto& res : resources_info)
	{
		std::visit
		(
			[this, &memory_block, res](auto&& v)
			{
				using T = std::decay<decltype(v)>;
				
				if constexpr (std::is_same_v<T, VkImage>)
				{
					vkBindImageMemory(this->dvc, *v, memory_block, res.bytes_offset);
				}
				else if constexpr (std::is_same_v<T, VkBuffer>)
				{
					vkBindBufferMemory(this->dvc, *v, memory_block, res.bytes_offset);
				}
			},
			res.res
		);
	}

	this->memory_manager_data.allocated_blocks[memory_block] = MemManagerClasses::MemBlockInfo
	{
		.size_summary = size_sum,
		.mem_flags = mem_flags,
		.resources = std::move(resources_info)
	};

	return static_cast<MemManagerClasses::MemBlock>(memory_block);
}