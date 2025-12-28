#include <internals/subsystems/proc/proc.hpp>

using ClassImpl = nxcraft::intern::subsystems::ProcRoot;

ClassImpl::ProcRoot()
{
	this->pinger = std::make_unique<Pinger>();
}

void ClassImpl::reply()
{
	this->last_reply_time = std::chrono::system_clock::now();
}

void ClassImpl::request_restart()
{
	this->make_restart = true;
}