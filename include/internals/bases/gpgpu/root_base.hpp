#pragma once
#include "../subsystem.hpp"
#include "../../subsystems/vid/vidroot.hpp"
#include "window_extension.hpp"
#include <optional>
#include <vector>

namespace nxcraft::intern::subsystems
{
	class GPGPU_Device
	{
	public:
		struct DeviceInfo
		{
			std::string name;
			std::string driver;
			uint64_t available_memory_bytes;
			uint64_t estimated_allocation_usage_bytes;
		};

		GPGPU_Device() = default;
		virtual ~GPGPU_Device() = default;

		virtual DeviceInfo retrieveInfo() = 0;
	};

	class GPGPU_RootBase : public SubsystemBase
	{
	public:
		enum class TextureFormatCompression
		{
			ASTC,
			BC7
		};
	protected:
		std::optional<TextureFormatCompression> compression_format;
		std::vector<GPGPU_WindowExtension*> wnd_extensions;

		virtual void clearWindowExtension(GPGPU_WindowExtension* ext) = 0;
		virtual void makeWindowExtension(GPGPU_WindowExtension* ext) = 0;
	public:
		GPGPU_RootBase() = default;
		virtual ~GPGPU_RootBase() = default;

		virtual void registerWindow(VidRoot::VidWindows::VidWndInfo& info) = 0;
		virtual void requestRecreation(VidRoot::VidWindows::VidWndInfo& info) = 0;
		virtual void handleUI(VidRoot::VidWindows::VidWndInfo& info) = 0;
		virtual void requestPresentation(VidRoot::VidWindows::VidWndInfo& info) = 0;
		const std::optional<TextureFormatCompression> getCompressionFormat() const;
	};
}