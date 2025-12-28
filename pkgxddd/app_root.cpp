#include "app_root.hpp"
#include <SDL3/SDL_events.h>

extern bool app_should_quit;

AppRoot::AppRoot()
{
	// Here place application's initialization code.
}
AppRoot::~AppRoot()
{
	// Here place application's termination code.
}
void AppRoot::main()
{
	SDL_Event events{};
	SDL_PollEvent(&events);
	if (events.type == SDL_EVENT_QUIT)
	{
		app_should_quit = true;
	}

	// Here is application's loop code.
}