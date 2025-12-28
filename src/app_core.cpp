#include <app_core.hpp>
#include <SDL3/SDL_events.h>

namespace nxcraft
{
	AppCore::AppCore()
	{

	}
	AppCore::~AppCore()
	{

	}
	void main()
	{
		AppCore::globs.app_executable_name = NXC_APP_NAME;

		SDL_Event events;
		
		while (SDL_PollEvent(&events))
		{
			if (events.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			{
				AppCore::globs.app_quit = true;
			}
		}
	}
}