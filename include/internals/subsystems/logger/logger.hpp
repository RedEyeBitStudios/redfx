#pragma once
#include "../../bases/subsystem.hpp"
#include <unordered_map>
#include <chrono>
#include <memory>

#define NXC_LOG_HELPER(...) nxcraft::intern::subsystems::LoggerRoot::Message(this, __VA_ARGS__)

namespace nxcraft::intern::subsystems
{
	class LoggerRoot : public SubsystemBase
	{
	private:
		std::string file_name;
		std::unordered_map<const void*, std::string> headers;
		std::chrono::system_clock::time_point startup_timepoint;
	public:
		class Message
		{
		public:
			enum Flags : uint16_t
			{
				MARK_AS_CRITICAL_ERROR = 1 << 0,
				SHOW_MESSAGE_BOX = 1 << 1,
				EXTEND_BY_ADDRESS = 1 << 2
			};

			Message() = delete;
			Message(const void* const address, std::string_view msg_content, Flags flags = Flags(0));
			virtual ~Message() = default;
		};

		friend class Message;

		LoggerRoot();
		void registerHeader(const void* const address, std::string_view header_content);
	};
}

nxcraft::intern::subsystems::LoggerRoot::Message::Flags operator|(const nxcraft::intern::subsystems::LoggerRoot::Message::Flags a, const nxcraft::intern::subsystems::LoggerRoot::Message::Flags b);