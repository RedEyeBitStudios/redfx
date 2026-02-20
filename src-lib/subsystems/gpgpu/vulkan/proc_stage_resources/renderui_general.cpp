#include "renderui_general.hpp"
#include <internals/subsystems/gpgpu/vulkan/functions.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_ProcessorStageResources_RenderUI_General;


ClassImpl::GPGPU_ProcessorStageResources_RenderUI_General(const GPGPU_Device_Vulkan::Commons& commons)
{
	const std::vector<VkAttachmentDescription2> attachments_info
	{
		VkAttachmentDescription2
		{
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
			.flags = 0,
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.samples = VK_SAMPLE_COUNT_8_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		},
		VkAttachmentDescription2
		{
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
			.flags = 0,
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		}
	};
	const std::vector<VkAttachmentReference2> output_attachment_references
	{
		VkAttachmentReference2
		{
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			.pNext = nullptr,
			.attachment = 0,
			.layout = attachments_info[0].initialLayout,
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
		}
	};
	const std::vector<VkAttachmentReference2> resolve_attachment_references
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
			.pResolveAttachments = resolve_attachment_references.data(),
			.pDepthStencilAttachment = nullptr,
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
	nxcraft::intern::vk::vkCreateRenderPass2KHR(commons.dvc, &renderpass_info, nullptr, &this->renderpass);
}
void ClassImpl::flush(const GPGPU_Device_Vulkan::Commons& commons)
{
	vkDestroyRenderPass(commons.dvc, this->renderpass, nullptr);
}