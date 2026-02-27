#include <internals/subsystems/gpgpu/root_vulkan.hpp>
#include <internals/subsystems/gpgpu/vulkan/window_extension.hpp>

#include "../../processor_stages/renderui_cull_elements.hpp"
#include "../../processor_stages/render_ui.hpp"

#include "../../proc_stage_resources/renderui_cull_elements.hpp"

using ClassImpl = nxcraft::intern::subsystems::GPGPU_RootVulkan;

void ClassImpl::updateUI(VidRoot::VidWindows::VidWndInfo& info)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu);
	const auto commons = this->primary_dvc->getCommons();
	const auto queue = this->primary_dvc->getQueue();
	auto next_frame = &wnd_ext->frames.ui->frames[wnd_ext->frames.ui->current_frame_id];

	GPGPU_ProcessorStageResources_RenderUI_CullElementsTmp resources_cull_elements;
	GPGPU_ProcessorStage_RenderUI_CullElements().process(info, this->primary_dvc.get(), std::any(std::ref(resources_cull_elements)));

	if (next_frame->state == GPGPU_FrameData_Vulkan::FrameState::FREE)
	{
		vkResetFences(commons.dvc, 1, &next_frame->cmd_fence);
		const VkCommandBufferBeginInfo cmd_info
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		vkBeginCommandBuffer(next_frame->cmd, &cmd_info);

		GPGPU_ProcessorStage_RenderUI().process(info, this->primary_dvc.get(), std::any(std::ref(resources_cull_elements)));

		vkEndCommandBuffer(next_frame->cmd);
		next_frame->state = GPGPU_FrameData_Vulkan::FrameState::RECORDED;

		const VkPipelineStageFlags stages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		const VkSubmitInfo submit_info
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.pWaitDstStageMask = &stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &next_frame->cmd,
			.signalSemaphoreCount = 0,
			.pSignalSemaphores = nullptr
		};
	
		vkQueueSubmit(this->primary_dvc->getQueue(), 1, &submit_info, next_frame->cmd_fence);
		next_frame->state = GPGPU_FrameData_Vulkan::FrameState::PENDING;
	}
	else if (next_frame->state == GPGPU_FrameData_Vulkan::FrameState::PENDING)
	{
		if (vkGetFenceStatus(commons.dvc, next_frame->cmd_fence) == VK_SUCCESS)
		{
			next_frame->state = GPGPU_FrameData_Vulkan::FrameState::FREE;
			wnd_ext->frames.ui->latest_frame = next_frame;
			wnd_ext->frames.ui->current_frame_id = wnd_ext->frames.ui->increment();
		}
	}
}