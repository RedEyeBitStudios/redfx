#include "renderui_imagebox.hpp"
#include <format>


using ClassImpl = nxcraft::intern::subsystems::GPGPU_ProcessorStageResources_RenderUI_ImageBox;

ClassImpl::GPGPU_ProcessorStageResources_RenderUI_ImageBox(const GPGPU_Device_Vulkan::Commons& commons, const GPGPU_ProcessorStageResources_RenderUI_General& renderui_generals)
{
	this->prepareDescriptors(commons, {});
	this->preparePipelineLayouts(commons, {});
	this->preparePipelines(commons, std::cref(renderui_generals));

	const VkSamplerCreateInfo sampler_info
	{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = 0.0f,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy = VK_FALSE,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0.0f,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = {},
		.unnormalizedCoordinates = VK_FALSE
	};
	vkCreateSampler(commons.dvc, &sampler_info, nullptr, &this->resources.sampler);
}
void ClassImpl::flush(const GPGPU_Device_Vulkan::Commons& commons)
{
	vkDestroyPipelineLayout(commons.dvc, this->render_pipeline.layout, nullptr);
	vkDestroyPipeline(commons.dvc, this->render_pipeline.handle, nullptr);
	vkDestroyDescriptorSetLayout(commons.dvc, this->render_pipeline.descriptors_layout, nullptr);
	vkDestroySampler(commons.dvc, this->resources.sampler, nullptr);
	vkDestroyDescriptorPool(commons.dvc, this->descriptors.allocation, nullptr);
}

void ClassImpl::prepareDescriptors(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data)
{
	const std::vector<VkDescriptorSetLayoutBinding> desc_layout_bind_infos
	{
		VkDescriptorSetLayoutBinding 
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 64,
			.stageFlags = this->shaders[1].stage_flags,
			.pImmutableSamplers = nullptr
		}
	};
	const VkDescriptorSetLayoutCreateInfo desc_layout_info
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.bindingCount = static_cast<uint32_t>(desc_layout_bind_infos.size()),
		.pBindings = desc_layout_bind_infos.data()
	};
	vkCreateDescriptorSetLayout(commons.dvc, &desc_layout_info, nullptr, &this->render_pipeline.descriptors_layout);

	const std::vector<VkDescriptorPoolSize> descs
	{
		VkDescriptorPoolSize
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 64
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
	vkCreateDescriptorPool(commons.dvc, &desc_allocation_info, nullptr, &this->descriptors.allocation);

	const VkDescriptorSetAllocateInfo desc_set_info
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = this->descriptors.allocation,
		.descriptorSetCount = 1,
		.pSetLayouts = &this->render_pipeline.descriptors_layout
	};
	vkAllocateDescriptorSets(commons.dvc, &desc_set_info, &this->descriptors.set);

	/*
	VkDescriptorUpdateTemplateEntry
	{
		.dstBinding = 0,
		.dstArrayElement = 0,
		.descriptorCount = 64,
		.descriptorType = 
	}
	const std::vector<VkDescriptorUpdateTemplateEntry> update_entries
	{

	};
	const VkDescriptorUpdateTemplateCreateInfo template_info
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.descriptorUpdateEntryCount = 64,
		.
	}
	vkCreateDescriptorUpdateTemplate(commons.dvc, )
	*/
}
void ClassImpl::preparePipelineLayouts(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data)
{
	VkPushConstantRange range
	{
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = 16
	};
	const VkPipelineLayoutCreateInfo pip_layout_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &this->render_pipeline.descriptors_layout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &range
	};
	vkCreatePipelineLayout(commons.dvc, &pip_layout_info, nullptr, &this->render_pipeline.layout);
}
void ClassImpl::preparePipelines(const GPGPU_Device_Vulkan::Commons& commons, std::any any_data)
{
	auto& generals_ref = std::any_cast<std::reference_wrapper<const GPGPU_ProcessorStageResources_RenderUI_General>>(any_data).get();

	std::vector<VkShaderModule> shaders;
	for (auto& info : this->shaders)
	{
		shaders.push_back(ClassImpl::makeShader(commons, info.file_name));
	}
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages_info;

	for (auto i = 0; i < shaders.size(); i++)
	{
		shader_stages_info.push_back
		(
			VkPipelineShaderStageCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stage = this->shaders[i].stage_flags,
				.module = shaders[i],
				.pName = "main",
				.pSpecializationInfo = nullptr
			}
		);
	}

	const VkPipelineVertexInputStateCreateInfo vertex_input_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr
	};
	const VkPipelineInputAssemblyStateCreateInfo ia_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		.primitiveRestartEnable = false
	};
	const VkPipelineViewportStateCreateInfo viewport_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.viewportCount = 1,
		.pViewports = nullptr,
		.scissorCount = 1,
		.pScissors = nullptr
	};
	const VkPipelineRasterizationStateCreateInfo rasterizer_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthClampEnable = false,
		.rasterizerDiscardEnable = false,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = false,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f
	};
	const VkPipelineMultisampleStateCreateInfo sampling_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT,
		.sampleShadingEnable = false,
		.minSampleShading = 0,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = false,
		.alphaToOneEnable = false
	};
	std::vector<VkPipelineColorBlendAttachmentState> attachments
	{
		VkPipelineColorBlendAttachmentState
		{
			.blendEnable = true,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor =  VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT
		}
	};
	const VkPipelineColorBlendStateCreateInfo blend_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.logicOpEnable = false,
		.logicOp = VK_LOGIC_OP_SET,
		.attachmentCount = static_cast<uint32_t>(attachments.size()),
		.pAttachments = attachments.data(),
		.blendConstants = {}
	};
	
	const std::vector<VkDynamicState> dynamics
	{
		VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
		VkDynamicState::VK_DYNAMIC_STATE_SCISSOR
	};
	const VkPipelineDynamicStateCreateInfo dynamics_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.dynamicStateCount = static_cast<uint32_t>(dynamics.size()),
		.pDynamicStates = dynamics.data()
	};
	const VkGraphicsPipelineCreateInfo pip_info
	{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stageCount = static_cast<uint32_t>(shader_stages_info.size()),
		.pStages = shader_stages_info.data(),
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &ia_info,
		.pTessellationState = nullptr,
		.pViewportState = &viewport_info,
		.pRasterizationState = &rasterizer_info,
		.pMultisampleState = &sampling_info,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &blend_info,
		.pDynamicState = &dynamics_info,
		.layout = this->render_pipeline.layout,
		.renderPass = generals_ref.renderpass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0
	};
	vkCreateGraphicsPipelines(commons.dvc, nullptr, 1, &pip_info, nullptr, &this->render_pipeline.handle);

	for (auto& shader_module : shaders)
	{
		vkDestroyShaderModule(commons.dvc, shader_module, nullptr);
	}
}