#include <internals/subsystems/gpgpu/root_vulkan.hpp>
#include <internals/subsystems/gpgpu/vulkan/window_extension.hpp>

#include "../../processor_stages/render_ui_colorbox.hpp"
#include "../../proc_stage_resources/stage_ui_colorbox.hpp"

using ClassImpl = nxcraft::intern::subsystems::GPGPU_RootVulkan;

void ClassImpl::updateUI(VidRoot::VidWindows::VidWndInfo& info)
{
	auto wnd_ext = static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu);
	const auto commons = this->primary_dvc->getCommons();
	const auto queue = this->primary_dvc->getQueue(GPGPU_Device_Vulkan::queue_name_graphics);
	auto frame = &wnd_ext->frames.ui->frames[wnd_ext->frames.ui->decrement()];

	if (vkGetFenceStatus(commons.dvc, frame->cmd_fence) != VK_SUCCESS)
	{
		// If UI rendering is not ready, then submit rendering next UI frame.
		return;
	}
	else
	{
		wnd_ext->frames.ui->current_frame_id = wnd_ext->frames.ui->increment();
		auto frame = &wnd_ext->frames.ui->frames[wnd_ext->frames.ui->increment()];

		vkResetFences(commons.dvc, 1, &frame->cmd_fence);
		
		const VkCommandBufferBeginInfo cmd_info
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		vkBeginCommandBuffer(frame->cmd, &cmd_info);

		GPGPU_ProcessorStage_RenderUI_ColorBox().process(info, this->primary_dvc->getProcessorStageData<GPGPU_ProcessorStageResources_UI_ColorBox>(), this->primary_dvc.get());

		vkEndCommandBuffer(frame->cmd);
		
		const VkPipelineStageFlags stages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		const VkSubmitInfo submit_info
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.pWaitDstStageMask = &stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &frame->cmd,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &wnd_ext->ui_render_semaphore
		};

		vkQueueSubmit(this->primary_dvc->getQueue(GPGPU_Device_Vulkan::queue_name_graphics).handle, 1, &submit_info, frame->cmd_fence);
	}
}