#include <internals/subsystems/logger/logger.hpp>
#include <subsystems.hpp>
#include <format>
#include <print>
#include <cassert>
#include <fstream>
#include <cassert>

#define COLOR_CODE_DEFAULT			"\033[0m"
#define COLOR_CODE_HEADER			"\033[38;2;0;160;255m"
#define COLOR_CODE_MSG_LEVEL_1		"\033[38;2;255;242;0m"
#define COLOR_CODE_CRITICAL_ERROR	"\033[38;2;255;0;0m"
#define COLOR_CODE_ADDRESS			"\033[38;2;150;215;255m"


using ClassImpl = nxcraft::intern::subsystems::LoggerRoot::Message;

extern bool critical_error_occured;

ClassImpl::Message(const void* const address, std::string_view msg_content, Flags flags)
{
	auto logger = &nxcraft::Subsystems::getSubsystem_Logger();

	assert(logger->headers.contains(address) && "Unregistered header.");
	
	const auto msg_header = logger->headers[address];
	const auto runtime_duration = std::chrono::system_clock::now() - logger->startup_timepoint;
	const auto formatted_duration = std::chrono::hh_mm_ss(runtime_duration);

	assert(msg_header.length() > 0 && "Invalid name.");

	const auto timepoint_msg = std::format
	(
		"{}:{}:{}",
		formatted_duration.minutes().count(),
		formatted_duration.seconds().count(),
		std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds(formatted_duration.subseconds().count())).count()
	);

#ifndef NDEBUG
	flags = flags | Flags::EXTEND_BY_ADDRESS;
#endif
	critical_error_occured = flags & Flags::MARK_AS_CRITICAL_ERROR;
	const auto color_code = (critical_error_occured ? COLOR_CODE_CRITICAL_ERROR : COLOR_CODE_DEFAULT);

	std::string tty_message;
	std::string log_message;
	if (flags & Flags::EXTEND_BY_ADDRESS)
	{
		tty_message = std::format
		(
			"{}{} {}{} {}{}{} {}",
			color_code,
			timepoint_msg,
			COLOR_CODE_ADDRESS,
			address,
			COLOR_CODE_HEADER,
			msg_header,
			COLOR_CODE_DEFAULT,
			msg_content
		);
		log_message = std::format
		(
			"{} {} {} {}",
			timepoint_msg,
			address,
			msg_header,
			msg_content
		);
	}
	else
	{
		tty_message = std::format
		(
			"{}{} {}{}{} {}",
			color_code,
			timepoint_msg,
			COLOR_CODE_HEADER,
			msg_header,
			COLOR_CODE_DEFAULT,
			msg_content
		);
		log_message = std::format
		(
			"{} {} {}",
			timepoint_msg,
			msg_header,
			msg_content
		);
	}
	std::println("{}{}", tty_message, COLOR_CODE_DEFAULT);

	std::fstream
	(
		logger->file_name,
		std::ios::out | std::ios::app
	) << log_message << "\n";

	if (flags & Flags::SHOW_MESSAGE_BOX)
	{
#ifdef __linux__
		std::string cmd;
		if (critical_error_occured)
		{
			cmd = std::format
			(
				"zenity --error --text='{}' --title='{}'",
				msg_content,
				"NexoraCraft - Error"
			);
		}
		else
		{
			cmd = std::format
			(
				"zenity --info --text='{}' --title='{}'",
				msg_content,
				"NexoraCraft - Info"
			);
		}
		auto h = popen(cmd.data(), "r");
		pclose(h);
#else
		#error "Unimplemented."
#endif
	}
}

nxcraft::intern::subsystems::LoggerRoot::Message::Flags operator|(const nxcraft::intern::subsystems::LoggerRoot::Message::Flags a, const nxcraft::intern::subsystems::LoggerRoot::Message::Flags b)
{
	return static_cast<nxcraft::intern::subsystems::LoggerRoot::Message::Flags>(uint16_t(a) | uint16_t(b));
}