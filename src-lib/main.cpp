#include <internals/../subsystems.hpp>
#include <internals/../app_core.hpp>

extern bool critical_error_occured;

int main()
{
	nxcraft::Subsystems subsystems;
	if (critical_error_occured) return -1;
	nxcraft::AppCore app;

	std::unique_ptr<nxcraft::TimePoint> delta_time;
	while (!app.globs.app_quit && !critical_error_occured)
	{
		delta_time.reset();
		subsystems.main();
		app.main();

		delta_time = std::make_unique<nxcraft::TimePoint>(nxcraft::delta_time_entry_name);
	}
}