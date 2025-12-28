#include <internals/subsystems/time/timeroot.hpp>
#include <internals/../subsystems.hpp>

using ClassImpl = nxcraft::intern::subsystems::TimeRoot::Point;

ClassImpl::Point(std::string_view entry_name)
{
	this->entry_name = entry_name;
	this->first_time_point = std::chrono::system_clock::now();
}
ClassImpl::~Point()
{
	nxcraft::Subsystems::getSubsystem_Time().getComponentRegistry()[this->entry_name] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - this->first_time_point);
}