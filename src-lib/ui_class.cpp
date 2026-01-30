#include <ui_class.hpp>

using ClassImpl = nxcraft::UI;

ClassImpl::UI(std::string_view page_name_in) : page_name{page_name_in}
{

}

std::vector<std::string> ClassImpl::process()
{
	return { this->page_name };
}

void ClassImpl::afterResizeEvent(const nexora_utils::math::ui16vec2 wh_px)
{
	
}