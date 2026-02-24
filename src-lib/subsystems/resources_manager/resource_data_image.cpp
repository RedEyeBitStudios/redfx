#include <internals/subsystems/resources_manager/resources_manager.hpp>
#include <subsystems.hpp>
#include <format>
#include <fstream>
#include <cassert>

#include <stb/stb_image.h>
//#include <tga/tga.h>

using ClassImpl = nxcraft::intern::subsystems::ResourcesManagerRoot::ResourceData_Image;

ClassImpl::ResourceData_Image(const ResourceManifest& manifest)
{
	int x, y, channels;
	auto pixels = stbi_load(manifest.implicits.asset_path.c_str(), &x, &y, &channels, 0);

	this->format = static_cast<Format>(channels + (stbi_is_hdr(manifest.implicits.asset_path.c_str()) ? 4 : 0));
	this->resolution = nexora_utils::math::ui16vec2(x, y);
	this->pixels_data.append_range(std::span(reinterpret_cast<uint8_t*>(pixels), x * y * channels));
	stbi_image_free(pixels);

	//
	//this->resolution = 
	//stbi_load()
	//stbi_load()
	//int x, y, comp;
	//auto pixels = stbi_load(, &x, &y, &comp, 0);
	

	//this->resolution = 
	//stbi__tga_load()
	/*
	TGA* tga = TGAOpen(, "rb");
	TGAData data{};
	TGAReadImage(tga, &data);
	const auto& tga_header = tga->hdr;

	if (TGA_IS_MAPPED(tga))
	{
		// TODO: Handle error. Mapped colors unsupported.
		return;
	}
	

	if (!TGA_IMGTYPE_AVAILABLE(tga))
	{
		// TODO: Handle error. No image data, unsupported.
		return;
	}
	else if (TGA_IMGTYPE_IS_ENCODED(tga))
	{
		// TODO: Handle error. RLE compression unsupported. OR: Implement RLE compression decoding.
		return;
	}

	this->resolution = nexora_utils::math::ui16vec2(tga_header.width, tga_header.height);
	
	const auto channels = (tga_header.depth / 8);
	//this->pixels_data.resize(static_cast<size_t>(tga_header.width) * static_cast<size_t>(tga_header.height) * channels);

	for (auto i = 0; i < static_cast<size_t>(tga_header.width) * static_cast<size_t>(tga_header.height) * channels; i++)
	{
		printf("byte id: %i\n", i);
		printf("%s\n", std::format("Byte id: {}; Byte: {}", i, data.img_data[i]).c_str());
	}


	//std::ranges::copy(std::span(reinterpret_cast<uint8_t*>(data.img_data), tga_header.width * tga_header.height * channels), this->pixels_data.begin());
	//this->pixels_data.append_range(std::span(reinterpret_cast<uint8_t*>(data.img_data), tga_header.width * tga_header.height * channels));
	

	TGAClose(tga);
	*/
}

nexora_utils::math::ui16vec2 ClassImpl::retrieveResolution() const
{
	return this->resolution;
}
std::span<uint8_t> ClassImpl::retrievePixelsData() const
{
	return this->pixels_data;
}
const ClassImpl::Format ClassImpl::retrieveFormat() const
{
	return this->format;
}