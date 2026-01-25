#include <utils/file_utils.hpp>
#include <filesystem>
#include <fstream>

using ClassImpl = nxcraft::FileUtils;

std::vector<char> ClassImpl::readWholeFile(std::string_view path)
{
	std::vector<char> buf(std::filesystem::file_size(path));
	std::ifstream(path.data(), std::ios::binary).read(buf.data(), buf.size());
	return buf;
}