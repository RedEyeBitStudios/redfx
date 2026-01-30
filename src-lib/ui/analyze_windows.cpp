#include <subsystems.hpp>
#include "ui_processor.hpp"

using ClassImpl = nxcraft::intern::ProcessorUI;

void ClassImpl::analyzeWindows()
{
	const auto keys = nxcraft::Subsystems::getSubsystem_Video().getComponent_Registry().retrieveAllRegistered();
	for (auto& key : keys)
	{
		auto window = nxcraft::Subsystems::getSubsystem_Video().getComponent_Registry().retrieve(key);
		if 
		(
			const auto flags = SDL_GetWindowFlags(window.handle);
			flags & SDL_WINDOW_MINIMIZED ||
			flags & SDL_WINDOW_HIDDEN
		)
		{
			continue;
		}
		ClassImpl::analyzePages(window);
	}
}