#pragma once
#include <string>

namespace nxcraft
{
	class AppCore
	{
	public:
		struct Globals
		{
			bool app_quit = false;
			std::string app_executable_name;
		};

		static Globals globs;

		AppCore();
		~AppCore();

		void main();
	};
}