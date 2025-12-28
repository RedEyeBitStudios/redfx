#include <internals/../subsystems.hpp>
#include <internals/../app_core.hpp>

int main()
{
	nxcraft::Subsystems subsystems;
	nxcraft::AppCore app;

	std::unique_ptr<nxcraft::TimePoint> delta_time;
	while (!nxcraft::AppCore::globs.app_quit)
	{
		delta_time.reset();
		subsystems.main();
		app.main();

		delta_time = std::make_unique<nxcraft::TimePoint>(nxcraft::delta_time_entry_name);
	}
}