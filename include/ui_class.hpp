#pragma once
#include <string>
#include <nx-utils/math/vec4.hpp>
#include <unordered_map>
#include <vector>
#include <array>

namespace nxcraft
{
	class UI
	{
	public:
		const std::string page_name;

		struct BoxProperties
		{
			enum BoxMask : uint8_t
			{
				IS_ACTIVE = 1u << 0,
				EVENT_TRIGGER = 1u << 1,
				ALIGNMENT_CENTER = 1u << 2,
				ALIGNMENT_RIGHT = 1u << 3
			};

			struct PositionPixel
			{
				nexora_utils::math::ui16vec2 position_px;
			};
			struct DimensionsPixel
			{
				nexora_utils::math::ui16vec2 wh_px;
			};
			struct ColorRGBA
			{
				uint32_t color_rgba;
			};
			struct LayersStack
			{
				uint8_t layer_id;
			};
			struct Mask
			{
				BoxMask mask;
			};
			struct Resource
			{
				std::array<char, 30> resource;
			};
			struct Multiplier
			{
				std::float16_t multiplier;
			};
			struct Text
			{
				std::array<char, 63> text;
			};
		};

		struct ColorBox : public BoxProperties::PositionPixel, BoxProperties::DimensionsPixel, BoxProperties::ColorRGBA, BoxProperties::LayersStack, BoxProperties::Mask
		{
			
		};
		struct TextBox : public BoxProperties::PositionPixel, BoxProperties::ColorRGBA, BoxProperties::LayersStack, BoxProperties::Mask, BoxProperties::Multiplier, BoxProperties::Text
		{

		};

		std::unordered_map<std::string, ColorBox> boxes;
		std::unordered_map<std::string, TextBox> text_boxes;
		uint8_t base_layer_id = 0;

		UI(std::string_view page_name);
		virtual ~UI() = default;

		virtual std::vector<std::string> process();
		virtual void afterResizeEvent(const nexora_utils::math::ui16vec2 wh_px);
	};
}