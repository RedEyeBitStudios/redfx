#pragma once
#include <string_view>

namespace nxcraft
{
	extern std::string_view app_executable_name;

	class AppCore
	{
	public:
		struct Globals
		{
			bool app_quit = false;
		};

		Globals globs;

		AppCore();
		~AppCore();

		void main();
	};
}
