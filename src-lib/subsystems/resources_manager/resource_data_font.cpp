#include <internals/subsystems/resources_manager/resources_manager.hpp>
#include <subsystems.hpp>
#include <format>
#include <fstream>

//#include <freetype2/ft2build.h>
//#include <freetype/freetype.h>

using ClassImpl = nxcraft::intern::subsystems::ResourcesManagerRoot::ResourceData_Font;

template<typename T>
static T getBufData(char*& buf_ptr)
{
	T cache = *reinterpret_cast<T*>(buf_ptr);
	buf_ptr += sizeof(T);
	return cache;
}

ClassImpl::ResourceData_Font(const ResourceManifest& manifest)
{
	this->font_name = std::get<ResourceManifest::Extensions::Extension_Font>(manifest.extension).name;

	std::vector<char> buf(manifest.general.file_size);
	std::ifstream(manifest.general.path.generic_string().data(), std::ios::binary).read(buf.data(), buf.size());

	char* buf_ptr = buf.data();

	for (char* buf_ptr = buf.data(); buf_ptr < buf.data() + buf.size(); )
	{
		ClassImpl::CharacterData character_data{};
		
		const uint32_t utf32_code = getBufData<uint32_t>(buf_ptr);
		const uint64_t vertices_count = getBufData<uint64_t>(buf_ptr);

		auto vertices_view = std::span(reinterpret_cast<nexora_utils::math::f16vec2*>(buf_ptr), vertices_count);
		character_data.vertices.append_range(vertices_view);
		buf_ptr += vertices_view.size_bytes();

		character_data.width = getBufData<std::float16_t>(buf_ptr);
		this->characters[utf32_code] = std::move(character_data);
	}



	/*
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
			//("Double-byte character: %X\n", first_byte);
		}
		else if (first_byte >= 0xE0 && first_byte <= 0xEF)
		{
			//("Triple-byte character: %X\n", first_byte);
		}
		else if (first_byte >= 0xF0 && first_byte <= 0xF4)
		{
			//("Four-byte character: %X\n", first_byte);
		}
		utf_code = FT_Get_Next_Char(face, utf_code, &c_index);
	}
	FT_Done_Face(face);
	FT_Done_FreeType(ft_library);
	*/
}
const ClassImpl::CharacterData* ClassImpl::operator[](const uint32_t utf32_code) const
{
	if (this->characters.contains(utf32_code))
	{
		return &this->characters[utf32_code];
	}
	return nullptr;
}
std::string_view ClassImpl::getFontName() const
{
	return this->font_name;
}
const std::unordered_map<uint32_t, ClassImpl::CharacterData>& ClassImpl::getCharacters() const
{
	return this->characters;
}