#pragma once
#include "../base/root.hpp"
#include <vector>
#include <span>
#include <cstdint>

namespace nxcraft::intern::subs
{
	class VidRoot : public SubsystemBase
	{
	public:
		struct VidMode
		{
			uint16_t width;
			uint16_t height;
			uint8_t refresh_rate;
		};
	private:
		std::vector<VidMode> fullscreen_modes;
		VidMode max_app_mode;
		VidMode system_mode;

		bool checkForVideoDriver();
		void loadDisplayModes();
	public:
		VidRoot();
		~VidRoot();

		enum class ModeType
		{
			MAX_APP_SPACE,
			SYSTEM
		};

		const VidMode& getVideoMode(const ModeType t = ModeType::MAX_APP_SPACE) const;
		const std::vector<VidMode>& getAvailableVideoModes() const;
	};
}