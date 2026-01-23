#include <internals/subsystems/logger/logger.hpp>
#include <format>
#include <ctime>
#include <format>
#include <print>
#include <filesystem>

using ClassImpl = nxcraft::intern::subsystems::LoggerRoot;

bool critical_error_occured = false;

ClassImpl::LoggerRoot()
{
	this->headers[nullptr] = "unnamed_header";
	this->startup_timepoint = std::chrono::system_clock::now();
	const auto timepoint = std::chrono::system_clock::to_time_t(this->startup_timepoint);
	this->file_name = std::format
	(
		"logs/{}.log",
		(
			std::stringstream() << std::put_time(std::localtime(&timepoint), "%Y_%m_%d_%H_%M_%S")
		).rdbuf()->str()
	);
}

void ClassImpl::registerHeader(const void* const address, std::string_view header_content)
{
	if (!this->headers.contains(address))
	{
		this->headers[address] = header_content;
	}
}