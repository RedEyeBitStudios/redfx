#pragma once
#include "vid_root.hpp"
#include "../uid/uid_manager.hpp"
#include <SDL3/SDL_video.h>

#include <unordered_map>
#include <array>

namespace nxcraft::intern::subs
{
	using VidWnd = SDL_Window*;
	using VidWndTitle = std::array<char, 32>;

	class VidManager
	{
	public:
		enum VidFlags : uint8_t
		{
			ACCEL 						= 1u << 0,
			FULLSCREEN 					= 1u << 1,
			UNDECORATED 				= 1u << 2,
			RESIZABLE 					= 1u << 3,
			TRANSPARENT_FRAMEBUFFER 	= 1u << 4,
			HIDDEN 						= 1u << 5
		};

		struct VidWndInfo
		{
			VidWnd		 			handle;
			SDL_Surface*			surf_handle;
			VidWndTitle				title;
			VidRoot::VidMode 		mode;
			VidFlags 				flags;
		};
	private:
		std::unordered_map<UniqID, VidWndInfo> wnd_pool;

		const std::vector<char> serialize(const VidWndInfo& wnd);
		std::pair<VidWndInfo, SDL_WindowFlags> deserialize(std::span<char> data);
	public:
		VidManager() = default;
		virtual ~VidManager();

		VidWndInfo& retrieve(const UniqID id);
		void updateFD(const UniqID id);
		void updateFDs();
	};
}