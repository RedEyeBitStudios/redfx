#include <internals/subsystems/resources_manager/resources_manager.hpp>
#include <subsystems.hpp>
#include <format>
#include <fstream>
#include <cassert>

#include <stb/stb_image.h>

using ClassImpl = nxcraft::intern::subsystems::ResourcesManagerRoot::ResourceData_Image;

ClassImpl::ResourceData_Image(const ResourceManifest& manifest)
{
	int x, y, channels;
	auto pixels = stbi_load(manifest.implicits.asset_path.c_str(), &x, &y, &channels, 0);

	// TODO: Only 8 and 32 bit color depth is supported.

	this->format = static_cast<Format>(channels + (stbi_is_hdr(manifest.implicits.asset_path.c_str()) ? 4 : 0));
	this->resolution = nexora_utils::math::ui16vec2(x, y);
	this->pixels_data.append_range(std::span(reinterpret_cast<uint8_t*>(pixels), x * y * channels));
	stbi_image_free(pixels);
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