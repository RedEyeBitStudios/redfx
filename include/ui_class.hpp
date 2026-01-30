#pragma once
#include <string>
#include <nx-utils/math/vec4.hpp>
#include <unordered_map>
#include <vector>

namespace nxcraft
{
	class UI
	{
	public:
		const std::string page_name;

		struct ColorBox
		{
			nexora_utils::math::ui16vec2 position_px;
			nexora_utils::math::ui16vec2 wh_px;
			uint32_t color_rgba;
			uint8_t depth;
			bool is_active;
		};

		std::unordered_map<std::string, ColorBox> boxes;

		UI(std::string_view page_name);
		virtual ~UI() = default;

		virtual std::vector<std::string> process();
		virtual void afterResizeEvent(const nexora_utils::math::ui16vec2 wh_px);
	};
}