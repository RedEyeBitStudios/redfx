#include <subsystems.hpp>
#include "ui_processor.hpp"

using ClassImpl = nxcraft::intern::ProcessorUI;

void ClassImpl::analyzePages(const subsystems::VidRoot::VidWindows::VidWndInfo& info)
{
	std::unordered_set<UI*> active_cache;
	if (!info.ui_ext->active.empty())
	{
		for (auto& page_ptr : info.ui_ext->active)
		{
			const auto page_names = page_ptr->process();
			for (auto& name : page_names)
			{
				active_cache.insert(info.ui_ext->registered[name].get());
			}
		}
		info.ui_ext->active = active_cache;
	}
}