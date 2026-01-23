#pragma once
#include <string>
#include <string_view>
#include <memory>

#define NXC_DECLARE_EXCEPTION(ExceptionName, ...) class ExceptionName : public intern::ErrorBase { public: ExceptionName(__VA_ARGS__); virtual ~ExceptionName() = default; }

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
	namespace intern = nxcraft::intern::err;

	using ErrorHolder = std::unique_ptr<intern::ErrorBase>;

	NXC_DECLARE_EXCEPTION(Vid_WindowAppend);
	NXC_DECLARE_EXCEPTION(Vid_WindowNameInUse);

	NXC_DECLARE_EXCEPTION(GPGPU_Vulkan_SystemConnectionFailure, const int result);
	NXC_DECLARE_EXCEPTION(GPGPU_Vulkan_SystemConnectionFailureSDL, std::string_view str);
	NXC_DECLARE_EXCEPTION(GPGPU_Vulkan_NoCompatibleDeviceFound);
	NXC_DECLARE_EXCEPTION(GPGPU_Vulkan_LogicalDeviceCreationFailed, const int result);
}