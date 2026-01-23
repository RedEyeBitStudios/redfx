#include <app_core.hpp>
#include <SDL3/SDL_events.h>

namespace nxcraft
{
	AppCore::AppCore()
	{
		AppCore::globs.app_executable_name = NXC_APP_NAME;
	}
	AppCore::~AppCore()
	{

	}
	void main()
	{
		SDL_Event events;
		
		while (SDL_PollEvent(&events))
		{
			if (events.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			{
				this->globs.app_quit = true;
			}
		}
	}
}