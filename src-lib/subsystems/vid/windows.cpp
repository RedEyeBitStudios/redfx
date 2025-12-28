#include <internals/subsystems/vid/vidroot.hpp>
#include <ranges>
#include <algorithm>

using ClassImpl = nxcraft::intern::subsystems::VidRoot::VidWindows;

ClassImpl::~VidWindows()
{
	for (auto& wnd : std::ranges::views::values(this->wnd_pool))
	{
		SDL_DestroyWindowSurface(wnd.handle);
		SDL_DestroyWindow(wnd.handle);
	}
}
ClassImpl::VidWndInfo& ClassImpl::retrieve(std::string_view wnd_name)
{
	return this->wnd_pool[wnd_name.data()];
}
ClassImpl::Err_Append ClassImpl::append(std::string_view wnd_name, ClassImpl::VidWndInfo info)
{
	if (info.handle != nullptr || info.accel_internals != nullptr || info.surf_handle != nullptr)
	{
		return Err_Append(nxcraft::err::Vid_WindowAppend("VidRoot-VidWindows-append-info-001: handle, surf_handle and accel_internals members must be nullptr."));
	}
	if (std::ranges::contains(std::ranges::views::keys(this->wnd_pool), wnd_name))
	{
		return Err_Append(nxcraft::err::Vid_WindowNameInUse("VidRoot-VidWindows-append-wnd_name-001: wnd_name must be unique name, which does not exist in this->wnd_pool."));
	}

	SDL_WindowFlags flags = 0;

	flags |= (info.flags & VidFlags::RESIZABLE ? SDL_WINDOW_RESIZABLE : 0);
	flags |= (info.flags & VidFlags::TRANSPARENT_FRAMEBUFFER ? SDL_WINDOW_TRANSPARENT : 0);
	flags |= (info.flags & VidFlags::UNDECORATED ? SDL_WINDOW_BORDERLESS : 0);
	flags |= (info.flags & VidFlags::FULLSCREEN ? SDL_WINDOW_FULLSCREEN : 0);
	flags |= (info.flags & VidFlags::HIDDEN ? SDL_WINDOW_HIDDEN : 0);

	info.title.back() = '\0';

	info.handle = SDL_CreateWindow(info.title.data(), info.mode.wh.x, info.mode.wh.y, flags);

	if (info.flags & VidFlags::ACCEL)
	{
		// Call window registration in accel subsystem.
	}

	this->wnd_pool[wnd_name.data()] = info;

	return Err_Append(nxcraft::err::NoError());
}