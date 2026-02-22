#include <internals/subsystems/resources_manager/resources_manager.hpp>
#include <subsystems.hpp>
#include <format>
#include <fstream>

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
	this->font_name = std::get<ResourceManifestClasses::Manifest_RedFXFont>(manifest.manifest).title;

	std::vector<char> buf(manifest.implicits.file_size);
	
	std::ifstream(manifest.implicits.asset_path.c_str(), std::ios::binary).read(buf.data(), buf.size());

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