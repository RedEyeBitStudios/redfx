#ifdef __linux__
	#include <internals/subsystems/proc/proc.hpp>
	#include <unistd.h>
	#include <spawn.h>
	#include <app_core.hpp>

using ClassImpl = nxcraft::intern::subsystems::ProcRoot;


ClassImpl::~ProcRoot()
{
	if (this->make_restart)
	{
		char** environ_local = environ;
		while (*environ_local != nullptr)
		{
			environ_local++;
		}
		
		int PID = 0;
		if (posix_spawn(&PID, AppCore::globs.app_executable_name.data(), nullptr, nullptr, nullptr, environ_local) == 0)
		{
			
		}
	}
}
#endif