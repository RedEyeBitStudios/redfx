#include <subsystems.hpp>
#include <internals/bases/subsystem.hpp>
#include <cstddef>
#include <SDL3/SDL_init.h>
#include "internals/subsystems/gpgpu/root_vulkan.hpp"

enum SubsystemName : size_t
{
	SUB_GPGPU,
	SUB_VIDEO,
	SUB_TIME,
	SUB_LOG
};

static std::array<nxcraft::intern::subsystems::SubsystemBase*, 4> subs {};

using ClassImpl = nxcraft::Subsystems;

SDL_WindowFlags additional_flags = 0;

ClassImpl::Subsystems()
{
	subs[SubsystemName::SUB_LOG] = new ClassImpl::LogRoot();
	subs[SubsystemName::SUB_TIME] = new ClassImpl::TimeRoot();
	subs[SubsystemName::SUB_VIDEO] = new ClassImpl::VidRoot();

	nxcraft::err::ErrorHolder err = nullptr;
	#ifdef __linux__
		subs[SubsystemName::SUB_GPGPU] = new nxcraft::intern::subsystems::GPGPU_RootVulkan(err);
		additional_flags = SDL_WINDOW_VULKAN;
	#elif _WIN64
		subs[SubsystemName::SUB_GPGPU] = new nxcraft::intern::subsystems::GPGPU_RootVulkan(err);
		additional_flags = SDL_WINDOW_VULKAN;
	#else
		#error "No GPGPU acceleration supported."
	#endif

	if (err != nullptr)
	{
		ClassImpl::LogRoot::Message
		(
			subs[SubsystemName::SUB_GPGPU],
			err->msg,
			ClassImpl::LogRoot::Message::MARK_AS_CRITICAL_ERROR | 
			ClassImpl::LogRoot::Message::SHOW_MESSAGE_BOX
		);
		return;
	}

	SDL_Init(SDL_INIT_EVENTS);
}
ClassImpl::~Subsystems()
{
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	for (auto& sub : subs)
	{
		delete sub;
	}
	SDL_Quit();
}
ClassImpl::TimeRoot& ClassImpl::getSubsystem_Time()
{
	return *static_cast<ClassImpl::TimeRoot*>(subs[SubsystemName::SUB_TIME]);
}
ClassImpl::VidRoot& ClassImpl::getSubsystem_Video()
{
	return *static_cast<ClassImpl::VidRoot*>(subs[SubsystemName::SUB_VIDEO]);
}
ClassImpl::AccelRoot& ClassImpl::getSubsystem_Accel()
{
	return *static_cast<ClassImpl::AccelRoot*>(subs[SubsystemName::SUB_GPGPU]);
}
void ClassImpl::main()
{
}
ClassImpl::LogRoot& ClassImpl::getSubsystem_Logger()
{
	return *static_cast<ClassImpl::LogRoot*>(subs[SubsystemName::SUB_LOG]);
}