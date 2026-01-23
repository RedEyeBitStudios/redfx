#include <internals/subsystems/vid/vidroot.hpp>
#include <subsystems.hpp>
#include <SDL3/SDL_init.h>
#include <span>

using ClassImpl = nxcraft::intern::subsystems::VidRoot;
 

ClassImpl::VidRoot()
{
	nxcraft::Subsystems::getSubsystem_Logger().registerHeader(this, "SubsystemVideo");
	SDL_Init(SDL_INIT_VIDEO);

	this->loadDisplayModes();
	NXC_LOG_HELPER("Initializing windows registry...");
	this->component_wnd_registry = std::make_unique<ClassImpl::VidWindows>();
}
ClassImpl::~VidRoot()
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
void ClassImpl::loadDisplayModes()
{
	NXC_LOG_HELPER("Loading display modes...");
	int modes_count = 0;
	auto modes = SDL_GetFullscreenDisplayModes(SDL_GetPrimaryDisplay(), &modes_count);
	this->fullscreen_modes.reserve(modes_count);

	for (auto& mode : std::span<SDL_DisplayMode*>(modes, modes_count))
	{
		if (mode->h > 720)
		{
			this->fullscreen_modes.push_back
			(
				ClassImpl::VidMode
				{
					.wh = nexora_utils::math::ui16vec2(mode->w, mode->h),
					.refresh_rate = static_cast<uint8_t>(mode->refresh_rate)
				}
			);
		}
	}
	SDL_free(modes);

	auto desktop_mode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
	this->system_mode = 
	{
		.wh = nexora_utils::math::ui16vec2(desktop_mode->w, desktop_mode->h),
		.refresh_rate = static_cast<uint8_t>(desktop_mode->refresh_rate)
	};

	SDL_Rect app_area;
	SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &app_area);
	this->max_app_mode = 
	{
		.position = nexora_utils::math::ui16vec2(app_area.x, app_area.y),
		.wh = nexora_utils::math::ui16vec2(app_area.w, app_area.h)
	};
}
const ClassImpl::VidMode& ClassImpl::getDesktopMode() const
{
	return this->system_mode;
}
ClassImpl::AppWorkArea ClassImpl::getMaxAppWorkArea() const
{
	return this->max_app_mode;
}
std::span<const ClassImpl::VidMode> ClassImpl::getAvailableVideoModes() const
{
	return this->fullscreen_modes;
}
ClassImpl::VidWindows& ClassImpl::getComponent_Registry() const
{
	return *this->component_wnd_registry.get();
}