#include <internals/subsystems/resources_manager/resources_manager.hpp>
#include <subsystems.hpp>
#include <format>

#include <freetype2/ft2build.h>
#include <freetype/freetype.h>

using ClassImpl = nxcraft::intern::subsystems::ResourcesManagerRoot::ResourceData_Font;

ClassImpl::ResourceData_Font(const ResourceManifest& manifest)
{
	FT_Library ft_library;
	FT_Init_FreeType(&ft_library);
	FT_Face face;
	FT_New_Face(ft_library, manifest.general.path.generic_string().data(), 0, &face);

	this->font_name = std::format("{} {}", std::string_view(face->family_name), std::string_view(face->style_name));

	FT_Set_Pixel_Sizes(face, 0, 48);
	uint32_t c_index = 0;
	uint64_t utf_code = FT_Get_First_Char(face, &c_index);
	while (c_index != 0)
	{
		const auto c_code = static_cast<uint32_t>(utf_code);
		FT_Load_Char(face, utf_code, FT_LOAD_RENDER);

		auto& character = this->characters[c_code];

		character = 
		{
			.size_px = static_cast<nexora_utils::math::ui8vec2>(nexora_utils::math::ui32vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows)),
			.bearing_px = static_cast<nexora_utils::math::i8vec2>(nexora_utils::math::i32vec2(face->glyph->bitmap_left, face->glyph->bitmap_top)),
			.offset_px = static_cast<uint16_t>(face->glyph->advance.x)
		};
		character.pixels.resize(character.size_px.x * character.size_px.y);

		std::ranges::copy(std::span<uint8_t>(reinterpret_cast<uint8_t*>(face->glyph->bitmap.buffer), character.size_px.x * character.size_px.y), character.pixels.begin());

		if (const auto first_byte = reinterpret_cast<uint8_t*>(&utf_code)[0]; first_byte > 0xC1 && first_byte <= 0xDF)
		{
			//printf("Double-byte character: %X\n", first_byte);
		}
		else if (first_byte >= 0xE0 && first_byte <= 0xEF)
		{
			//printf("Triple-byte character: %X\n", first_byte);
		}
		else if (first_byte >= 0xF0 && first_byte <= 0xF4)
		{
			//printf("Four-byte character: %X\n", first_byte);
		}
		utf_code = FT_Get_Next_Char(face, utf_code, &c_index);
	}
	FT_Done_Face(face);
	FT_Done_FreeType(ft_library);
}
const ClassImpl::CharacterData* ClassImpl::operator[](const uint32_t utf_code) const
{
	return &this->characters[utf_code];
}
std::string_view ClassImpl::getFontName() const
{
	return this->font_name;
}