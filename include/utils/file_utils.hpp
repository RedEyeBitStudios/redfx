#pragma once
#include <vector>
#include <string_view>
#include <span>

namespace nxcraft
{
	class FileUtils
	{
	public:
		[[nodiscard]] static std::vector<char> readWholeFile(std::string_view path);
	};
}