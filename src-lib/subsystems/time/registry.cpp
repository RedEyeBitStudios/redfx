#include <internals/subsystems/time/timeroot.hpp>

using ClassImpl = nxcraft::intern::subsystems::TimeRoot::Registry;

ClassImpl::Registry()
{
	this->data[nxcraft::delta_time_entry_name] = std::chrono::milliseconds(0);
}
std::chrono::milliseconds& ClassImpl::operator[](std::string_view entry_name)
{
	return this->data[entry_name.data()];
}