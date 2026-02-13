#include <internals/subsystems/gpgpu/root_vulkan.hpp>
#include <internals/subsystems/gpgpu/vulkan/window_extension.hpp>

#include "../../processor_stages/render_ui_colorbox.hpp"
#include "../../proc_stage_resources/stage_ui_colorbox.hpp"

using ClassImpl = nxcraft::intern::subsystems::GPGPU_RootVulkan;

void ClassImpl::updateUI(VidRoot::VidWindows::VidWndInfo& info)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu);
	const auto commons = this->primary_dvc->getCommons();
	const auto queue = this->primary_dvc->getQueue();
	auto next_frame = &wnd_ext->frames.ui->frames[wnd_ext->frames.ui->current_frame_id];

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

		GPGPU_ProcessorStage_RenderUI_ColorBox().process(info, this->primary_dvc->getProcessorStageData<GPGPU_ProcessorStageResources_UI_ColorBox>(), this->primary_dvc.get());

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
	
	

	/*
	if (wnd_ext->frames.ui->recorded.empty())
	{
		auto next_frame = &wnd_ext->frames.ui->frames[wnd_ext->frames.ui->increment()];

		const VkCommandBufferBeginInfo cmd_info
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		vkBeginCommandBuffer(next_frame->cmd, &cmd_info);

		GPGPU_ProcessorStage_RenderUI_ColorBox().process(info, this->primary_dvc->getProcessorStageData<GPGPU_ProcessorStageResources_UI_ColorBox>(), this->primary_dvc.get());

		vkEndCommandBuffer(next_frame->cmd);

		wnd_ext->frames.ui->recorded.push_back(next_frame);
	}

	if (vkGetFenceStatus(commons.dvc, frame->cmd_fence) == VK_SUCCESS)
	{
		// If fence is signaled, that means frame is ready. Then submit next cmd is possible.
		wnd_ext->frames.ui->current_frame_id = wnd_ext->frames.ui->increment();

		if (!wnd_ext->frames.ui->recorded.empty())
		{
			auto frame_to_submit = wnd_ext->frames.ui->recorded.front();			

			

			wnd_ext->frames.ui->recorded.erase(wnd_ext->frames.ui->recorded.begin());
		}
	}
		*/
}