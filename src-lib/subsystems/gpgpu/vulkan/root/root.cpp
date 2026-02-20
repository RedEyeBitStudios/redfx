#include <internals/subsystems/gpgpu/root_vulkan.hpp>
#include <internals/subsystems/gpgpu/vulkan/window_extension.hpp>
#include <cassert>
#include <SDL3/SDL_vulkan.h>
#include <span>
#include <algorithm>
#include <subsystems.hpp>
#include <internals/subsystems/gpgpu/vulkan/functions.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_RootVulkan;

#define NXC_PCI_VENDOR_AMD 0x1002
#define NXC_PCI_VENDOR_NVIDIA 0x10de
#define NXC_PCI_VENDOR_INTEL 0x8086
#define NXC_PCI_VENDOR_ARM 0x13b5

constexpr const static uint32_t vulkan_version = VK_API_VERSION_1_1;

extern std::array<const char*, 5> required_dvc_extensions_names;

VkBool32 ClassImpl::debugCallback
(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* user_data
)
{
	static bool error_already_hit = false;
	if (callback_data->pMessage != nullptr)
	{
		nxcraft::Subsystems::LogRoot::Message(user_data, std::format("{}", callback_data->pMessage), (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ? LoggerRoot::Message::Flags::MARK_AS_CRITICAL_ERROR : LoggerRoot::Message::Flags(0)));
		if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT && !error_already_hit)
		{
			nxcraft::Subsystems::LogRoot::Message(user_data, "Vulkan API internal client error detected.", LoggerRoot::Message::Flags::SHOW_MESSAGE_BOX | LoggerRoot::Message::Flags::MARK_AS_CRITICAL_ERROR);
			error_already_hit = true;
		}
	}
	return VK_FALSE;
}

ClassImpl::GPGPU_RootVulkan(nxcraft::err::ErrorHolder& err)
{
	nxcraft::Subsystems::getSubsystem_Logger().registerHeader(this, "SubsystemGPGPU");
	nxcraft::Subsystems::LogRoot::Message(this, std::format("Initialization started.")); 
	nxcraft::Subsystems::LogRoot::Message(this, "Selected API: Vulkan");

	const VkApplicationInfo app_info
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr,
		.pApplicationName = nullptr,
		.applicationVersion = 0,
		.pEngineName = "NexoraCraft",
		.engineVersion = VK_MAKE_VERSION(26, 1, 0),
		.apiVersion = vulkan_version
	};
	
	auto check_layer = [](std::string_view layer_name) -> const char*
	{
		uint32_t layers_count = 0;
		vkEnumerateInstanceLayerProperties(&layers_count, nullptr);
		std::vector<VkLayerProperties> layers(layers_count);
		vkEnumerateInstanceLayerProperties(&layers_count, layers.data());

		for (auto& layer : layers)
		{
			if (std::string_view(layer.layerName) == layer_name)
			{
				return layer_name.data();
			}
		}

		return nullptr;
	};

	std::vector<const char*> layers;
	std::vector<const char*> extensions;

	bool validation_enabled = false;
	
	if (auto result = check_layer("VK_LAYER_KHRONOS_validation"); result != nullptr)
	{
		validation_enabled = true;
		nxcraft::Subsystems::LogRoot::Message(this, "Validation layer enabled.");
		nxcraft::Subsystems::getSubsystem_Logger().registerHeader(&this->sys_debug_msg, "VULKAN-API");
		layers.push_back(result);
		extensions.push_back("VK_EXT_debug_utils");
	}

	if
	(
		![&extensions]() -> bool
		{
			uint32_t extensions_count = 0;
			auto extensions_sdl = SDL_Vulkan_GetInstanceExtensions(&extensions_count);

			if (extensions_sdl != nullptr)
			{
				extensions.insert_range(extensions.begin(), std::span<const char* const>(extensions_sdl, extensions_count));
				return true;
			}
			return false;
		}()
	)
	{
		err.reset(new nxcraft::err::GPGPU_Vulkan_SystemConnectionFailureSDL(SDL_GetError()));
		return;
	}

	const VkInstanceCreateInfo create_info
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = static_cast<uint32_t>(layers.size()),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data()
	};

	if (auto result = vkCreateInstance(&create_info, nullptr, &this->sys_con); result != VK_SUCCESS)
	{
		err.reset(new nxcraft::err::GPGPU_Vulkan_SystemConnectionFailure(static_cast<int>(result)));
		return;
	}
	else
	{
		nxcraft::intern::vk::vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetInstanceProcAddr(this->sys_con, "vkGetBufferDeviceAddressKHR"));
		nxcraft::intern::vk::vkCreateRenderPass2KHR = reinterpret_cast<PFN_vkCreateRenderPass2KHR>(vkGetInstanceProcAddr(this->sys_con, "vkCreateRenderPass2KHR"));

		if (validation_enabled)
		{
			nxcraft::intern::vk::vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->sys_con, "vkCreateDebugUtilsMessengerEXT"));
			nxcraft::intern::vk::vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(this->sys_con, "vkDestroyDebugUtilsMessengerEXT"));

			const VkDebugUtilsMessengerCreateInfoEXT debug_create_info
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.pNext = nullptr,
				.flags = 0,
				.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
				.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				.pfnUserCallback = &this->debugCallback,
				.pUserData = &this->sys_debug_msg
			};
			nxcraft::intern::vk::vkCreateDebugUtilsMessengerEXT(this->sys_con, &debug_create_info, nullptr, &this->sys_debug_msg);
		}
		nxcraft::Subsystems::LogRoot::Message(this, "Checking for available devices...");

		// If supported device is present, continue initialization.
		if (const auto ph_dvc = this->enumDevice(); ph_dvc != VK_NULL_HANDLE)
		{
			constexpr const char* fmt = "Selected primary device:\nName: {}\nDriver version: {}";
			this->primary_dvc = std::make_unique<GPGPU_Device_Vulkan>(ph_dvc, err);
			const auto primary_info = this->primary_dvc->retrieveInfo();

			nxcraft::Subsystems::LogRoot::Message(this, std::format(fmt, primary_info.name, primary_info.driver));
		}
		else
		{
			err.reset(new nxcraft::err::GPGPU_Vulkan_NoCompatibleDeviceFound());
			return;
		}
	}
}
ClassImpl::~GPGPU_RootVulkan()
{
	auto& vid_reg_root = nxcraft::Subsystems::getSubsystem_Video().getComponent_Registry();

	for (auto& entry : vid_reg_root.retrieveAllRegistered())
	{
		auto gpgpu_ext = vid_reg_root.retrieve(entry).gpgpu;
		this->clearWindowExtension(gpgpu_ext);
		delete gpgpu_ext;
	}
	this->primary_dvc.reset();

	if (this->sys_debug_msg != VK_NULL_HANDLE)
	{
		nxcraft::intern::vk::vkDestroyDebugUtilsMessengerEXT(this->sys_con, this->sys_debug_msg, nullptr);
	}
	if (this->sys_con)
	{
		vkDestroyInstance(this->sys_con, nullptr);
	}
}
VkPhysicalDevice ClassImpl::enumDevice()
{
	std::vector<VkPhysicalDevice> ph_devices;
	{
		uint32_t dvc_count = 0;
		vkEnumeratePhysicalDevices(this->sys_con, &dvc_count, nullptr);
		ph_devices.resize(dvc_count);
		vkEnumeratePhysicalDevices(this->sys_con, &dvc_count, ph_devices.data());
	}
	   
	for (auto& ph_dvc : ph_devices)
	{
		// Retrieve and check basic device properties.
		VkPhysicalDeviceProperties2 props
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
			.pNext = nullptr
		};

		vkGetPhysicalDeviceProperties2(ph_dvc, &props);
		
		if (props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
		{
			// Only hardware acceleration acceptable.
			continue;
		}
		if (VK_VERSION_MAJOR(props.properties.apiVersion) <= VK_VERSION_MAJOR(vulkan_version))
		{
			if (VK_VERSION_MINOR(props.properties.apiVersion) <= VK_VERSION_MINOR(vulkan_version))
			{
				// At least specific Vulkan feature level must be supported.
				continue;
			}
		}

		// Look for proper queues support.
		if 
		(
			![&ph_dvc, this]() -> bool
			{
				const auto queues = [&ph_dvc]() -> std::vector<VkQueueFamilyProperties2>
				{
					uint32_t queues_count = 0;
					vkGetPhysicalDeviceQueueFamilyProperties2(ph_dvc, &queues_count, nullptr);
					std::vector<VkQueueFamilyProperties2> queues
					(
						queues_count,
						VkQueueFamilyProperties2
						{
							.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2
						}
					);
					vkGetPhysicalDeviceQueueFamilyProperties2(ph_dvc, &queues_count, queues.data());
					return queues;
				}();

				constexpr const std::array<VkQueueFlagBits, 2> required_queue_bits
				{
					VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT,
					VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT
				};
				std::unordered_set<VkQueueFlagBits> bits;
				for (auto& queue : queues)
				{
					for (auto required_bit : required_queue_bits)
					{
						if (queue.queueFamilyProperties.queueFlags & required_bit)
						{
							bits.insert(required_bit);
						}
					}
				}

				return bits.size() == required_queue_bits.size();
			}()
		)
		{
			continue;
		}
		
		// Look for proper extensions support.
		if 
		(
			![&ph_dvc]() -> bool
			{ 
				const auto dvc_extensions = [&ph_dvc]() -> std::vector<VkExtensionProperties>
				{
					uint32_t count = 0;
					vkEnumerateDeviceExtensionProperties(ph_dvc, nullptr, &count, nullptr);
					std::vector<VkExtensionProperties> extensions(count);
					vkEnumerateDeviceExtensionProperties(ph_dvc, nullptr, &count, extensions.data());
					return extensions;
				}();
				std::unordered_set<std::string_view> supported_extensions_cache;
				
				for (auto ext : dvc_extensions)
				{
					for (auto required_ext_name : required_dvc_extensions_names)
					{
						if (std::string_view(required_ext_name) == std::string_view(ext.extensionName))
						{
							supported_extensions_cache.insert(ext.extensionName);
						}
					}
				}
				return supported_extensions_cache.size() == required_dvc_extensions_names.size();
			}()
		)
		{
			continue;
		}

		VkPhysicalDeviceFeatures core_features;
		if 
		(
			const auto result = [&ph_dvc, &core_features]() -> bool
			{
				vkGetPhysicalDeviceFeatures(ph_dvc, &core_features);

				const std::vector<VkBool32*> core_features_ptrs
				{
					&core_features.tessellationShader,
					&core_features.fillModeNonSolid,
					&core_features.samplerAnisotropy,
					&core_features.shaderInt16
				};

				for (const auto ptr : core_features_ptrs)
				{
					if (!*ptr)
					{
						return false;
					}
				}
				return true;
			}()
		)
		{
			if (core_features.textureCompressionASTC_LDR)
			{
				this->compression_format.emplace(ClassImpl::TextureFormatCompression::ASTC);
			}
			else if (core_features.textureCompressionBC)
			{
				this->compression_format.emplace(ClassImpl::TextureFormatCompression::BC7);
			}
			else
			{
				continue;
			}
		}
		else
		{
			this->compression_format.reset();
			continue;
		}

		return ph_dvc;
	}
	return VK_NULL_HANDLE;
}
void ClassImpl::registerWindow(VidRoot::VidWindows::VidWndInfo& info)
{
	GPGPU_WindowExtension_Vulkan* wnd_ext = new GPGPU_WindowExtension_Vulkan();
	info.gpgpu = wnd_ext;

	SDL_Vulkan_CreateSurface(info.handle, this->sys_con, nullptr, &wnd_ext->surface);

	this->recreateSwapchain(info, wnd_ext);
	this->makeWindowExtension(wnd_ext);
}
void ClassImpl::handleWindow(VidRoot::VidWindows::VidWndInfo& info)
{
	this->updateUI(info);
	if (reinterpret_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu)->frames.ui->latest_frame != nullptr)
	{
		this->requestPresentation(info);
	}
	this->primary_dvc->submitTransfer();
}
void ClassImpl::requestRecreation(VidRoot::VidWindows::VidWndInfo& info)
{
	this->recreateSwapchain(info, static_cast<GPGPU_WindowExtension_Vulkan*>(info.gpgpu));
}