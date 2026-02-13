#pragma once
#include <cstdint>
#include <unordered_map>
#include <nx-utils/math/vec2.hpp>
#include <string>

namespace nxcraft::intern::subsystems
{
	struct VidText_CharacterData
	{
		static constexpr const float mp = 1.0f / 64.0f;
		nexora_utils::math::ui8vec2 size_px;
		nexora_utils::math::i8vec2 bearing_px;
		uint16_t offset_px;
		std::vector<uint8_t> pixels;
	};

	using VidText_Font = std::unordered_map<uint32_t, VidText_CharacterData>; 
}