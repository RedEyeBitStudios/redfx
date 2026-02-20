#include <ui_class.hpp>
#include <span>
#include <subsystems.hpp>

using ClassImpl = nxcraft::UI;

ClassImpl::UI(std::string_view page_name_in) : page_name{page_name_in}
{
	auto resource = nxcraft::Subsystems::getSubsystem_ResourcesManager().retrieveResourceView(this->page_name);
	static_cast<const nxcraft::Subsystems::ResourcesManagerRoot::ResourceData_RedFXUI*>(resource->data.get())->fill(*this);
}

std::vector<std::string> ClassImpl::process()
{
	return { this->page_name };
}

void ClassImpl::afterResizeEvent(const nexora_utils::math::ui16vec2 wh_px)
{
	
}