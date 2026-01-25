#pragma once
#include "../../bases/gpgpu/root_base.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <cassert>
#include <unordered_map>
#include "../../bases/err.hpp"
#include "vulkan/devices.hpp"
#include <span>
#include <chrono>

namespace nxcraft::intern::subsystems
{
	class GPGPU_RootVulkan final : public GPGPU_RootBase
	{
	private:
		static VkBool32 debugCallback
		(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
			void* user_data
		);

		VkInstance sys_con = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT sys_debug_msg = VK_NULL_HANDLE;

		std::unique_ptr<GPGPU_Device_Vulkan> primary_dvc;

		VkPhysicalDevice enumDevice();
		void recreateSwapchain(VidRoot::VidWindows::VidWndInfo& info, GPGPU_WindowExtension_Vulkan* wnd_ext);
		void requestPresentation(VidRoot::VidWindows::VidWndInfo& info);
		void updateUI(VidRoot::VidWindows::VidWndInfo& info);
	protected:
		void clearWindowExtension(GPGPU_WindowExtension* ext) override;
		void makeWindowExtension(GPGPU_WindowExtension* ext) override;

		std::chrono::nanoseconds max_presentation_rate;
	public:
		GPGPU_RootVulkan(nxcraft::err::ErrorHolder& err);
		virtual ~GPGPU_RootVulkan();

		void registerWindow(VidRoot::VidWindows::VidWndInfo& info) override;
		void requestRecreation(VidRoot::VidWindows::VidWndInfo& info) override;
		void handleWindow(VidRoot::VidWindows::VidWndInfo& info) override;
	};
}

//void handlePresentation(VidRoot::VidWindows::VidWndInfo& info);