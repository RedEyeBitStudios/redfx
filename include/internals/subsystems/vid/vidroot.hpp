#pragma once
#include "../../bases/subsystem.hpp"
#include <internals/bases/err.hpp>
#include <nx-utils/math/vec2.hpp>
#include <cstdint>
#include <vector>
#include <span>
#include <array>
#include <SDL3/SDL_video.h>
#include <unordered_map>

namespace nxcraft::intern::subsystems
{
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
				void* accel_internals;
			};

			using Err_Append = std::variant
			<
				nxcraft::err::NoError,
				nxcraft::err::Vid_WindowAppend,
				nxcraft::err::Vid_WindowNameInUse
			>;
		private:
			std::unordered_map<std::string, VidWndInfo> wnd_pool;
		public:
			VidWindows() = default;
			~VidWindows();

			VidWndInfo& retrieve(std::string_view wnd_name);
			Err_Append append(std::string_view wnd_name, VidWndInfo info);
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