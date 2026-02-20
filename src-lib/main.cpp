#include <internals/../subsystems.hpp>
#include <internals/../app_core.hpp>

extern bool critical_error_occured;

#include <filesystem>
#include <fstream>
#include <format>
#include <unordered_set>
#include <vector>

int main()
{
	nxcraft::Subsystems subsystems;
	if (critical_error_occured) return -1;
	nxcraft::AppCore app;

	while (!app.globs.app_quit && !critical_error_occured)
	{
		nxcraft::TimePoint delta_time(nxcraft::delta_time_entry_name);
		subsystems.main();
		app.main();
	}
}