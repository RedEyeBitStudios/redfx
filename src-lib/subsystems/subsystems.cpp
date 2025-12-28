#include <subsystems.hpp>
#include <internals/bases/subsystem.hpp>
#include <cstddef>
#include <SDL3/SDL_init.h>

enum SubsystemName : size_t
{
	SUB_VIDEO,
	SUB_TIME
};

static std::array<nxcraft::intern::subsystems::SubsystemBase*, 2> subs {};

namespace nxcraft
{
	Subsystems::Subsystems()
	{
		subs[SubsystemName::SUB_TIME] = new nxcraft::intern::subsystems::TimeRoot();
		subs[SubsystemName::SUB_VIDEO] = new nxcraft::intern::subsystems::VidRoot();

		SDL_Init(SDL_INIT_EVENTS);
	}
	Subsystems::~Subsystems()
	{
		SDL_QuitSubSystem(SDL_INIT_EVENTS);
		for (auto& sub : subs)
		{
			delete sub;
		}
		SDL_Quit();
	}

	intern::subsystems::TimeRoot& Subsystems::getSubsystem_Time()
	{
		return *static_cast<intern::subsystems::TimeRoot*>(subs[SubsystemName::SUB_TIME]);
	}
	intern::subsystems::VidRoot& Subsystems::getSubsystem_Video()
	{
		return *static_cast<intern::subsystems::VidRoot*>(subs[SubsystemName::SUB_VIDEO]);
	}
	void Subsystems::main()
	{
	}
}