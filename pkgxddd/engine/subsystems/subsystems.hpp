#pragma once
#include "accel/accel_root.hpp"
#include "vid/vid_root.hpp"
#include "vid/vid_manager.hpp"
#include "uid/uid_manager.hpp"

namespace nxcraft
{
	class Subsystems final
	{
	public:
		static intern::subs::UID_Manager*	 	const 	getHandle_UIDManager();
		static intern::subs::AccelRoot* 		const	getHandle_AccelRoot();
		static intern::subs::VidRoot* 			const	getHandle_VidRoot();
		//static intern::subs::VidManager* 		const	getHandle_VidManager();

		Subsystems();
		~Subsystems();
	};
}