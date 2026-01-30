#pragma once
#include "../../bases/subsystem.hpp"
#include "../../bases/err.hpp"
#include "../../bases/gpgpu/window_extension.hpp"
#include "../../../ui_class.hpp"
#include <nx-utils/math/vec2.hpp>
#include <cstdint>
#include <vector>
#include <span>
#include <SDL3/SDL_video.h>
#include <unordered_map>
#include <ranges>
#include <type_traits>
#include <unordered_set>

namespace nxcraft::intern::subsystems
{
	struct ContainerUI
	{
		std::unordered_map<std::string, std::unique_ptr<UI>> registered;
		std::unordered_set<UI*> active;
	};

	class VidRoot : public SubsystemBase
	{
	public:
		struct VidMode
		{
			nexora_utils::math::ui16vec2 wh;
			uint8_t refresh_rate;
		};
		struct AppWorkArea
		{
			nexora_utils::math::ui16vec2 position;
			nexora_utils::math::ui16vec2 wh;
		};

		class VidWindows
		{
		public:
			using Handle = SDL_Window*;
			using SurfaceHandle = SDL_Surface*;
			using Title = std::array<char, 32>;

			enum VidFlags : uint8_t
			{
				ACCEL = 1u << 0,
				FULLSCREEN = 1u << 1,
				UNDECORATED = 1u << 2,
				RESIZABLE = 1u << 3,
				TRANSPARENT_FRAMEBUFFER = 1u << 4,
				HIDDEN = 1u << 5
			};

			struct VidWndInfo
			{
				Handle handle;
				SurfaceHandle surf_handle;
				Title title;
				VidMode mode;
				VidFlags flags;
				GPGPU_WindowExtension* gpgpu;
				ContainerUI* ui_ext;
			};

		private:
			std::unordered_map<std::string, VidWndInfo> wnd_pool;
		public:
			VidWindows();
			~VidWindows();

			using KeysSet = decltype(std::views::keys(wnd_pool));

			VidWndInfo& retrieve(std::string_view wnd_name);
			nxcraft::err::ErrorHolder append(std::string_view wnd_name, VidWndInfo info);
			KeysSet retrieveAllRegistered();
			template<typename T>
			void appendUI(std::string_view wnd_name, T& ui_ptr, bool as_active = false) requires (std::is_base_of_v<UI, T>)
			{
				auto wnd = &this->retrieve(wnd_name);
				UI* ptr = static_cast<UI*>(&ui_ptr);
				wnd->ui_ext->registered[std::move(ptr->page_name)].reset(ptr);
				if (as_active)
				{
					wnd->ui_ext->active.insert(ptr);
				}
			}
		};

	private:
		std::vector<VidMode> fullscreen_modes;
		AppWorkArea max_app_mode;
		VidMode system_mode;
		std::unique_ptr<VidWindows> component_wnd_registry;
		void loadDisplayModes();

	public:
		const VidMode& getDesktopMode() const;
		AppWorkArea getMaxAppWorkArea() const;

		std::span<const VidMode> getAvailableVideoModes() const;

		VidWindows& getComponent_Registry() const;

		VidRoot();
		virtual ~VidRoot();
	};
}