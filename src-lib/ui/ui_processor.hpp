#pragma once
#include <internals/subsystems/vid/vidroot.hpp>

namespace nxcraft::intern
{
	class ProcessorUI final
	{
	private:
		static void analyzePages(const subsystems::VidRoot::VidWindows::VidWndInfo& info);
		static void analyzeWindows();

	public:
		static void process();
	};
}