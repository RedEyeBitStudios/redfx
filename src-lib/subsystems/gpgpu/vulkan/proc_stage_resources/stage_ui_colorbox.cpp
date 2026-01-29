#include "stage_ui_colorbox.hpp"
#include <format>
#include <internals/subsystems/gpgpu/vulkan/functions.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_ProcessorStageResources_UI_ColorBox;

namespace nxcraft::intern::subsystems
{
	static void prepareDescriptors(ClassImpl* ptr, const GPGPU_Device_Vulkan::Commons& commons)
	{
		/*
		const std::vector<VkDescriptorSetLayoutBinding> desc_layout_bind_infos
		{
			VkDescriptorSetLayoutBinding 
			{
				.binding = 5,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = 1,
				.stageFlags = ptr->colorbox_shader.stage_flags,
				.pImmutableSamplers = nullptr
			},
			VkDescriptorSetLayoutBinding 
			{
				.binding = 6,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = 1,
				.stageFlags = ptr->colorbox_shader.stage_flags,
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
		vkCreateDescriptorSetLayout(commons.dvc, &desc_layout_info, nullptr, &ptr->desc_layout);

		const std::vector<VkDescriptorPoolSize> descs
		{
			VkDescriptorPoolSize
			{
				.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = 2
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
		*/
	}

	static void prepareRenderPasses(ClassImpl* ptr, const GPGPU_Device_Vulkan::Commons& commons)
	{
		const std::vector<VkAttachmentDescription2> attachments_info
		{
			VkAttachmentDescription2
			{
				.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.flags = 0,
				.format = VK_FORMAT_D16_UNORM,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
			},
			VkAttachmentDescription2
			{
				.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
				.flags = 0,
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			}
		};

		const VkAttachmentReference2 depth_attachment_reference
		{
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			.pNext = nullptr,
			.attachment = 0,
			.layout = attachments_info[0].initialLayout,
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT
		};

		const std::vector<VkAttachmentReference2> output_attachment_references
		{
			VkAttachmentReference2
			{
				.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
				.pNext = nullptr,
				.attachment = 1,
				.layout = attachments_info[1].initialLayout,
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
			}
		};

		const std::vector<VkSubpassDescription2> subpasses_info
		{
			VkSubpassDescription2
			{
				.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
				.pNext = nullptr,
				.flags = 0,
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.viewMask = 0,
				.inputAttachmentCount = 0,
				.pInputAttachments = nullptr,
				.colorAttachmentCount = static_cast<uint32_t>(output_attachment_references.size()),
				.pColorAttachments = output_attachment_references.data(),
				.pResolveAttachments = nullptr,
				.pDepthStencilAttachment = &depth_attachment_reference,
				.preserveAttachmentCount = 0,
				.pPreserveAttachments = nullptr
			}
		};

		const VkRenderPassCreateInfo2 renderpass_info
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR,
			.pNext = nullptr,
			.flags = 0,
			.attachmentCount = static_cast<uint32_t>(attachments_info.size()),
			.pAttachments = attachments_info.data(),
			.subpassCount = static_cast<uint32_t>(subpasses_info.size()),
			.pSubpasses = subpasses_info.data(),
			.dependencyCount = 0,
			.pDependencies = nullptr,
			.correlatedViewMaskCount = 0,
			.pCorrelatedViewMasks = nullptr
		};

		nxcraft::intern::vk::vkCreateRenderPass2KHR(commons.dvc, &renderpass_info, nullptr, &ptr->render_pipeline.pass);
	}
}

ClassImpl::GPGPU_ProcessorStageResources_UI_ColorBox(const GPGPU_Device_Vulkan::Commons& commons)
{
	std::vector<VkShaderModule> shaders;
	VkPushConstantRange range
	{
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = 16
	};
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
	
	const VkPipelineLayoutCreateInfo pip_layout_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 0,
		.pSetLayouts = nullptr,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &range
	};
	vkCreatePipelineLayout(commons.dvc, &pip_layout_info, nullptr, &this->render_pipeline.layout);
	prepareRenderPasses(this, commons);

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
	const VkPipelineTessellationStateCreateInfo tessellation_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.patchControlPoints = 0
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
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = false,
		.minSampleShading = 0,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = false,
		.alphaToOneEnable = false
	};
	const VkPipelineDepthStencilStateCreateInfo depth_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = VK_COMPARE_OP_GREATER,
		.depthBoundsTestEnable = false,
		.stencilTestEnable = false,
		.front = VK_STENCIL_OP_KEEP,
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 65536.0f
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
		.pDepthStencilState = &depth_info,
		.pColorBlendState = &blend_info,
		.pDynamicState = &dynamics_info,
		.layout = this->render_pipeline.layout,
		.renderPass = this->render_pipeline.pass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0
	};
	vkCreateGraphicsPipelines(commons.dvc, nullptr, 1, &pip_info, nullptr, &this->render_pipeline.handle);

	for (auto& shader_module : shaders)
	{
		vkDestroyShaderModule(commons.dvc, shader_module, nullptr);
	}
	/*
	prepareDescriptors(this, commons);
	

	const VkPushConstantRange range
	{
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset = 0,
		.size = 16
	};

	const VkPipelineLayoutCreateInfo pip_layout_info
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &this->desc_layout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &range
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
	

	const std::vector<VkDescriptorUpdateTemplateEntry> desc_update_template_entries
	{
		VkDescriptorUpdateTemplateEntry
		{
			.dstBinding = 5,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.offset = 0,
			.stride = 1
		},
		VkDescriptorUpdateTemplateEntry
		{
			.dstBinding = 6,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.offset = sizeof(VkDescriptorImageInfo),
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
	*/
}
void ClassImpl::flush(const GPGPU_Device_Vulkan::Commons& commons)
{
	vkDestroyRenderPass(commons.dvc, this->render_pipeline.pass, nullptr);
	vkDestroyPipelineLayout(commons.dvc, this->render_pipeline.layout, nullptr);
	vkDestroyPipeline(commons.dvc, this->render_pipeline.handle, nullptr);
	/*
	vkDestroyDescriptorSetLayout(commons.dvc, this->desc_layout, nullptr);
	vkDestroyPipelineLayout(commons.dvc, this->pipeline_layout_comp, nullptr);
	vkDestroyPipeline(commons.dvc, this->pipeline_comp, nullptr);
	vkDestroyDescriptorPool(commons.dvc, this->descs_pool, nullptr);
	vkDestroyDescriptorUpdateTemplate(commons.dvc, this->desc_update_template, nullptr);
	*/
}