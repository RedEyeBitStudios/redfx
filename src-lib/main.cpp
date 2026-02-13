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

	
	const auto windows = subsystems.getSubsystem_Video().getComponent_Registry().retrieveAllRegistered();

	for (auto& wnd : windows)
	{
		auto x = subsystems.getSubsystem_Video().getComponent_Registry().retrieve(wnd).ui_ext;

		for (auto& entry : x->registered)
		{
			auto res_view = subsystems.getSubsystem_ResourcesManager().retrieveResourceView(entry.first);
			if (std::get<nxcraft::Subsystems::ResourcesManagerRoot::ResourceManifest::Extensions::Extension_RedFXUI>(res_view->manifest_ptr->extension).active_on_init)
			{
				x->active.insert(entry.second.get());
			}
		}
	}
		
	while (!app.globs.app_quit && !critical_error_occured)
	{
		nxcraft::TimePoint delta_time(nxcraft::delta_time_entry_name);
		subsystems.main();
		app.main();
	}
}