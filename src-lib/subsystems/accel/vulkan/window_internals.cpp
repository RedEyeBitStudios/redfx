#include <subsystems.hpp>
#include <internals/subsystems/gpgpu/root_vulkan.hpp>
/*
using ClassImpl = nxcraft::intern::subsystems::AccelWindowData;
using ClassImpl2 = ClassImpl::FrameData;
using ClassImpl3 = ClassImpl::UI_ResourcesFrame;
using GPGPU_RootVulkan = nxcraft::intern::subsystems::GPGPU_RootVulkan;

ClassImpl::AccelWindowData(AccelWindowData&& obj)
{
	this->data_per_frame = std::move(obj.data_per_frame);
	this->ui = std::move(obj.ui);
	this->surface = obj.surface;
	this->swp = obj.swp;

	obj.surface = VK_NULL_HANDLE;
	obj.swp = VK_NULL_HANDLE;
}
ClassImpl::~AccelWindowData()
{
	const auto accel = static_cast<GPGPU_RootVulkan*>(&nxcraft::Subsystems::getSubsystem_Accel());
	this->data_per_frame.clear();
	vkDestroySwapchainKHR(accel->dvc_main->handle, this->swp, nullptr);
	vkDestroySurfaceKHR(accel->sys_con, this->surface, nullptr);
}
ClassImpl2::~FrameData()
{
	const auto accel = static_cast<GPGPU_RootVulkan*>(&nxcraft::Subsystems::getSubsystem_Accel());
	vkDestroyCommandPool(accel->dvc_main->handle, this->cmd_allocation, nullptr);
	vkDestroyImageView(accel->dvc_main->handle, this->view, nullptr);
}
ClassImpl3::~UI_ResourcesFrame()
{
	const auto dvc = static_cast<GPGPU_RootVulkan*>(&nxcraft::Subsystems::getSubsystem_Accel())->dvc_main->handle;

	vkDestroyImageView(dvc, this->framebuffer_view, nullptr);
	vkDestroyImage(dvc, this->framebuffer, nullptr);
}
*/