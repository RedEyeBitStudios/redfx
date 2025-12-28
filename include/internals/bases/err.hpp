#pragma once
#include <variant>
#include <string>
#include <string_view>

namespace nxcraft::intern::err
{
	class ErrorBase
	{
	public:
		std::string msg = "null";

		ErrorBase() = delete;
		ErrorBase(std::string_view msg)
		{
			this->msg = msg;
		}

		virtual ~ErrorBase() = default;
	};
}
namespace nxcraft::err
{
	using NoError = std::monostate;
}
namespace nxcraft::err
{
	namespace intern = nxcraft::intern::err;

	class Vid_WindowAppend : public intern::ErrorBase
	{
	public:
		Vid_WindowAppend(const char* str) : intern::ErrorBase(str) {}
		virtual ~Vid_WindowAppend() = default;
	};
	class Vid_WindowNameInUse : public intern::ErrorBase
	{
	public:
		Vid_WindowNameInUse(const char* str) : intern::ErrorBase(str) {}
		virtual ~Vid_WindowNameInUse() = default;
	};
}