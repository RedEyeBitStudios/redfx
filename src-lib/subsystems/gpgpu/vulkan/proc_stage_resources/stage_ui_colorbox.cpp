#include "stage_ui_colorbox.hpp"
#include <format>
#include <utils/file_utils.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_ProcessorStageResources_UI_ColorBox;

namespace nxcraft::intern::subsystems
{
	static void prepareDescriptors(ClassImpl* ptr, const GPGPU_Device_Vulkan::Commons& commons)
	{
		const VkDescriptorSetLayoutBinding desc_layout_bind_info
		{
			.binding = 5,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.descriptorCount = 1,
			.stageFlags = ptr->colorbox_shader.stage_flags,
			.pImmutableSamplers = nullptr
		};
		const VkDescriptorSetLayoutCreateInfo desc_layout_info
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.bindingCount = 1,
			.pBindings = &desc_layout_bind_info
		};
		vkCreateDescriptorSetLayout(commons.dvc, &desc_layout_info, nullptr, &ptr->desc_layout);

		const std::vector<VkDescriptorPoolSize> descs
		{
			VkDescriptorPoolSize
			{
				.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = 1
			}
		};
		const VkDescriptorPoolCreateInfo desc_allocation_info
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			.maxSets = 1,
			.poolSizeCount = static_cast<uint32_t>(descs.size()),
			.pPoolSizes = descs.data()
		};
		vkCreateDescriptorPool(commons.dvc, &desc_allocation_info, nullptr, &ptr->descs_pool);

		VkDescriptorSetAllocateInfo desc_set_info
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = ptr->descs_pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &ptr->desc_layout
		};
		vkAllocateDescriptorSets(commons.dvc, &desc_set_info, &ptr->desc_set);
	}
}



ClassImpl::GPGPU_ProcessorStageResources_UI_ColorBox(const GPGPU_Device_Vulkan::Commons& commons)
{
	prepareDescriptors(this, commons);
	const auto file_path = std::format("{}/{}", "shaders/vulkan", this->colorbox_shader.file_name);
	auto shader_code_buf = nxcraft::FileUtils::readWholeFile(file_path);
	
	VkShaderModule ui_comp_shader;
	const VkShaderModuleCreateInfo shader_info
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = shader_code_buf.size(),
		.pCode = reinterpret_cast<uint32_t*>(shader_code_buf.data())
	};
	vkCreateShaderModule(commons.dvc, &shader_info, nullptr, &ui_comp_shader);

	const VkPipelineLayoutCreateInfo pip_layout_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &this->desc_layout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr
	};
	vkCreatePipelineLayout(commons.dvc, &pip_layout_info, nullptr, &this->pipeline_layout_comp);

	const VkComputePipelineCreateInfo pip_info
	{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = 
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = this->colorbox_shader.stage_flags,
			.module = ui_comp_shader,
			.pName = "main",
			.pSpecializationInfo = nullptr
		},
		.layout = this->pipeline_layout_comp,
		.basePipelineHandle = nullptr,
		.basePipelineIndex = 0
	};
	vkCreateComputePipelines(commons.dvc, nullptr, 1, &pip_info, nullptr, &this->pipeline_comp);
	vkDestroyShaderModule(commons.dvc, ui_comp_shader, nullptr);

	std::vector<VkDescriptorUpdateTemplateEntry> desc_update_template_entries
	{
		VkDescriptorUpdateTemplateEntry
		{
			.dstBinding = 5,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.offset = 0,
			.stride = 1
		}
	};

	const VkDescriptorUpdateTemplateCreateInfo desc_template_info
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.descriptorUpdateEntryCount = static_cast<uint32_t>(desc_update_template_entries.size()),
		.pDescriptorUpdateEntries = desc_update_template_entries.data(),
		.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
		.descriptorSetLayout = this->desc_layout,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE,
		.pipelineLayout = this->pipeline_layout_comp,
		.set = 0
	};

	vkCreateDescriptorUpdateTemplate(commons.dvc, &desc_template_info, nullptr, &this->desc_update_template);
}
void ClassImpl::flush(const GPGPU_Device_Vulkan::Commons& commons)
{
	vkDestroyDescriptorSetLayout(commons.dvc, this->desc_layout, nullptr);
	vkDestroyPipelineLayout(commons.dvc, this->pipeline_layout_comp, nullptr);
	vkDestroyPipeline(commons.dvc, this->pipeline_comp, nullptr);
	vkDestroyDescriptorPool(commons.dvc, this->descs_pool, nullptr);
	vkDestroyDescriptorUpdateTemplate(commons.dvc, this->desc_update_template, nullptr);
}