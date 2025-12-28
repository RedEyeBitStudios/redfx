#pragma once
#include "internals/subsystems/time/timeroot.hpp"
#include "internals/subsystems/vid/vidroot.hpp"

namespace nxcraft
{
	class Subsystems final
	{
	public:
		static intern::subsystems::TimeRoot& getSubsystem_Time();
		static intern::subsystems::VidRoot& getSubsystem_Video();
		Subsystems();
		~Subsystems();

		void main();
	};
}