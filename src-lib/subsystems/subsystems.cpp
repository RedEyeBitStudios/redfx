#include <subsystems.hpp>
#include <internals/bases/subsystem.hpp>
#include <cstddef>
#include <SDL3/SDL_init.h>
#include "internals/subsystems/gpgpu/root_vulkan.hpp"
#include "../ui/ui_processor.hpp"
#include <memory>

enum class SubsystemName : uint16_t
{
	SUB_GPGPU,
	SUB_VIDEO,
	SUB_TIME,
	SUB_LOG,
	SUB_RESOURCE_MANAGER
};

static std::unordered_map<SubsystemName, std::unique_ptr<nxcraft::intern::subsystems::SubsystemBase>> subs;

using ClassImpl = nxcraft::Subsystems;

SDL_WindowFlags additional_flags = 0;

ClassImpl::Subsystems()
{
	subs[SubsystemName::SUB_LOG] = std::move(std::make_unique<ClassImpl::LogRoot>());
	subs[SubsystemName::SUB_TIME] = std::move(std::make_unique<ClassImpl::TimeRoot>());
	subs[SubsystemName::SUB_VIDEO] = std::move(std::make_unique<ClassImpl::VidRoot>());
	subs[SubsystemName::SUB_RESOURCE_MANAGER] = std::move(std::make_unique<nxcraft::intern::subsystems::ResourcesManagerRoot>());

	nxcraft::err::ErrorHolder err = nullptr;
	#ifdef __linux__
		subs[SubsystemName::SUB_GPGPU] = std::move(std::make_unique<nxcraft::intern::subsystems::GPGPU_RootVulkan>(err));
		additional_flags = SDL_WINDOW_VULKAN;
	#elif _WIN64
		subs[SubsystemName::SUB_GPGPU] = std::move(std::make_unique<nxcraft::intern::subsystems::GPGPU_RootVulkan>(err));
		additional_flags = SDL_WINDOW_VULKAN;
	#else
		#error "No GPGPU acceleration supported."
	#endif

	if (err != nullptr)
	{
		ClassImpl::LogRoot::Message
		(
			subs[SubsystemName::SUB_GPGPU].get(),
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
	subs.clear();
	SDL_Quit();
}
ClassImpl::TimeRoot& ClassImpl::getSubsystem_Time()
{
	return *static_cast<ClassImpl::TimeRoot*>(subs[SubsystemName::SUB_TIME].get());
}
ClassImpl::VidRoot& ClassImpl::getSubsystem_Video()
{
	return *static_cast<ClassImpl::VidRoot*>(subs[SubsystemName::SUB_VIDEO].get());
}
ClassImpl::AccelRoot& ClassImpl::getSubsystem_Accel()
{
	return *static_cast<ClassImpl::AccelRoot*>(subs[SubsystemName::SUB_GPGPU].get());
}
ClassImpl::LogRoot& ClassImpl::getSubsystem_Logger()
{
	return *static_cast<ClassImpl::LogRoot*>(subs[SubsystemName::SUB_LOG].get());
}
ClassImpl::ResourcesManagerRoot& ClassImpl::getSubsystem_ResourcesManager()
{
	return *static_cast<ClassImpl::ResourcesManagerRoot*>(subs[SubsystemName::SUB_RESOURCE_MANAGER].get());
}
void ClassImpl::main()
{
	nxcraft::intern::ProcessorUI::process();
	getSubsystem_ResourcesManager().submitQueue();
}