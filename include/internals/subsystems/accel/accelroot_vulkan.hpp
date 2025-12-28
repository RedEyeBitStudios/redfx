#pragma once
#include <internals/bases/accel_base.hpp>

namespace nxcraft::intern::subsystems
{
	class AccelRoot_Vulkan : public AccelRootBase
	{
	public:
		AccelRoot_Vulkan();
		virtual ~AccelRoot_Vulkan();

		void registerWindow(VidRoot::VidWindows::VidWndInfo& info) override;
	};
}