#pragma once
#include "internals/subsystems/time/timeroot.hpp"
#include "internals/subsystems/vid/vidroot.hpp"
#include "internals/bases/gpgpu/root_base.hpp"
#include "internals/subsystems/logger/logger.hpp"
#include "internals/subsystems/resources_manager/resources_manager.hpp"

namespace nxcraft
{
	class Subsystems final
	{
	public:
		using TimeRoot = intern::subsystems::TimeRoot;
		using VidRoot = intern::subsystems::VidRoot;
		using AccelRoot = intern::subsystems::GPGPU_RootBase;
		using LogRoot = intern::subsystems::LoggerRoot;
		using ResourcesManagerRoot = intern::subsystems::ResourcesManagerRoot;

		static TimeRoot& getSubsystem_Time();
		static VidRoot& getSubsystem_Video();
		static AccelRoot& getSubsystem_Accel();
		static LogRoot& getSubsystem_Logger();
		static ResourcesManagerRoot& getSubsystem_ResourcesManager();

		Subsystems();
		~Subsystems();

		void main();
	};
}