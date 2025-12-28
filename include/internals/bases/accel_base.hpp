#pragma once
#include "subsystem.hpp"
#include "../subsystems/vid/vidroot.hpp"

namespace nxcraft::intern::subsystems
{
	class AccelRootBase : public SubsystemBase
	{
	public:
		AccelRootBase() = default;
		virtual ~AccelRootBase() = default;

		virtual void registerWindow(VidRoot::VidWindows::VidWndInfo& info) = 0;
	};
}