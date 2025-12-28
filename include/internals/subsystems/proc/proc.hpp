#pragma once
#include "../../bases/subsystem.hpp"
#include <chrono>

namespace nxcraft::intern::subsystems
{
	class ProcRoot : public SubsystemBase
	{
	private:
		bool make_restart = false;

		std::chrono::system_clock::time_point last_reply_time;

		class Pinger
		{
		public:
			std::chrono::system_clock::time_point last_ping_time;

			void ping();

			Pinger();
			~Pinger();
		};
		
		std::unique_ptr<Pinger> pinger = nullptr;
	public:
		ProcRoot();
		~ProcRoot();

		void reply();
		void request_restart();
	};
}