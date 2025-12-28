#include <internals/subsystems/time/timeroot.hpp>

using ClassImpl = nxcraft::intern::subsystems::TimeRoot;

ClassImpl::TimeRoot()
{
	this->component_registry = std::make_unique<ClassImpl::Registry>();
}
ClassImpl::~TimeRoot()
{

}
ClassImpl::Registry& ClassImpl::getComponentRegistry()
{
	return *this->component_registry.get();
}