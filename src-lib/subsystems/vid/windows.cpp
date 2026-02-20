#include <internals/subsystems/vid/vidroot.hpp>
#include <ranges>
#include <algorithm>
#include <subsystems.hpp>
#include <format>
#include <type_traits>

extern SDL_WindowFlags additional_flags;

using ClassImpl = nxcraft::intern::subsystems::VidRoot::VidWindows;

ClassImpl::VidWindows()
{
	nxcraft::Subsystems::getSubsystem_Logger().registerHeader(this, "SubsystemVideo::WindowsRegistry");
}

ClassImpl::~VidWindows()
{
	for (auto& wnd : std::ranges::views::values(this->wnd_pool))
	{
		SDL_DestroyWindowSurface(wnd.handle);
		SDL_DestroyWindow(wnd.handle);
		delete wnd.ui_ext;
	}
}
ClassImpl::VidWndInfo& ClassImpl::retrieve(std::string_view wnd_name)
{
	return this->wnd_pool[wnd_name.data()];
}
nxcraft::err::ErrorHolder ClassImpl::append(std::string_view wnd_name, ClassImpl::VidWndInfo info)
{
	NXC_LOG_HELPER(std::format("Adding a window: {}", wnd_name));
	if (info.handle != nullptr || info.gpgpu != nullptr || info.surf_handle != nullptr)
	{
		return std::make_unique<nxcraft::err::Vid_WindowAppend>();
	}
	if (std::ranges::contains(std::ranges::views::keys(this->wnd_pool), wnd_name))
	{
		return std::make_unique<nxcraft::err::Vid_WindowNameInUse>();
	}

	SDL_WindowFlags flags = 0;

	flags |= (info.flags & VidFlags::RESIZABLE ? SDL_WINDOW_RESIZABLE : 0);
	flags |= (info.flags & VidFlags::TRANSPARENT_FRAMEBUFFER ? SDL_WINDOW_TRANSPARENT : 0);
	flags |= (info.flags & VidFlags::UNDECORATED ? SDL_WINDOW_BORDERLESS : 0);
	flags |= (info.flags & VidFlags::FULLSCREEN ? SDL_WINDOW_FULLSCREEN : 0);
	flags |= (info.flags & VidFlags::HIDDEN ? SDL_WINDOW_HIDDEN : 0);
	flags |= additional_flags;

	info.title.back() = '\0';

	info.handle = SDL_CreateWindow(info.title.data(), info.mode.wh.x, info.mode.wh.y, flags);

	if (info.flags & VidFlags::ACCEL)
	{
		NXC_LOG_HELPER(std::format("Registering window: {}", wnd_name));
		nxcraft::Subsystems::getSubsystem_Accel().registerWindow(info);
	}

	info.ui_ext = new ContainerUI();

	this->wnd_pool[wnd_name.data()] = std::move(info);

	return nullptr;
}
ClassImpl::KeysSet ClassImpl::retrieveAllRegistered()
{
	return std::views::keys(this->wnd_pool);
}

void ClassImpl::appendUI(std::string_view wnd_name, UI* ui_ptr)
{
	auto wnd = &this->retrieve(wnd_name);
	UI* ptr = static_cast<UI*>(ui_ptr);
	wnd->ui_ext->registered[ptr->page_name].reset(ptr);

	auto res_view = nxcraft::Subsystems::getSubsystem_ResourcesManager().retrieveResourceView(ui_ptr->page_name);
	if (std::get<nxcraft::Subsystems::ResourcesManagerRoot::ResourceManifest::Extensions::Extension_RedFXUI>(res_view->manifest_ptr->extension).active_on_init)
	{
		wnd->ui_ext->active.insert(ptr);
	}
}