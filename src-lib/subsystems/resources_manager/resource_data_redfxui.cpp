#include <internals/subsystems/resources_manager/resources_manager.hpp>
#include <subsystems.hpp>
#include <format>
#include <fstream>
#include <filesystem>

using ClassImpl = nxcraft::intern::subsystems::ResourcesManagerRoot::ResourceData_RedFXUI;

template<typename T>
static T getBufData(char*& buf_ptr)
{
	T cache = *reinterpret_cast<T*>(buf_ptr);
	buf_ptr += sizeof(T);
	return cache;
}

ClassImpl::ResourceData_RedFXUI(const ResourceManifest& manifest)
{
	std::vector<char> tmp_buf(manifest.implicits.file_size);
	std::ifstream(manifest.implicits.asset_path.c_str(), std::ios::binary).read(tmp_buf.data(), std::span(tmp_buf).size_bytes());

	char* buf_ptr = tmp_buf.data();
	this->base_layer_id = getBufData<uint8_t>(buf_ptr);

	const auto cbc = getBufData<uint64_t>(buf_ptr);
	for (uint64_t i = 0; i < cbc; i++)
	{
		struct cBoxData
		{
			char name[16];
			UI::ColorBox data;
		};
		const auto cbox_data = getBufData<cBoxData>(buf_ptr);
		std::string str(cbox_data.name, sizeof(cbox_data.name));
		std::erase(str, 0);
		this->boxes[str] = std::move(cbox_data.data);
	}
}

void ClassImpl::fill(nxcraft::UI& ui) const
{
	ui.boxes = this->boxes;
	ui.text_boxes = this->text_boxes;
	ui.base_layer_id = this->base_layer_id;
}