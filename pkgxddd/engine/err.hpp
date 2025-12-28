#pragma once
#include <string>
#include <string_view>
#include <variant>
#include <format>

namespace nxcraft::err::intern
{
	class Error
	{
	private:
	    Error() = default;
	public:
	    std::string msg;

	    Error(std::string_view msg)
	    {
	        this->msg = msg;
	    }
	    virtual ~Error() = default;
	};
}

namespace nxcraft::err
{
	using NoError = std::monostate;

	class VulkanDriverUnsupported : public intern::Error
	{
	public:
		VulkanDriverUnsupported() : Error("Vulkan driver unsupported.") {}
		virtual ~VulkanDriverUnsupported() = default;
	};
	class VulkanValidationUnsupported : public intern::Error
	{
	public:
		VulkanValidationUnsupported() : Error("Vulkan validation unsupported.") {}
		virtual ~VulkanValidationUnsupported() = default;
	};
	class VulkanExtensionUnsupported : public intern::Error
	{
	public:
		VulkanExtensionUnsupported() : Error("Vulkan extension unsupported.") {}
		virtual ~VulkanExtensionUnsupported() = default;
	};
	class AppendUIDFailed : public intern::Error
	{
	public:
		AppendUIDFailed(std::string_view msg) : Error(msg) {}
		virtual ~AppendUIDFailed() = default;
	};

	class VulkanDevice_ExtensionUnsupported : public intern::Error
	{
	public:
		VulkanDevice_ExtensionUnsupported(std::string_view name = "") : Error(std::format("Unsupported device extension.{}", name)) {}
		virtual ~VulkanDevice_ExtensionUnsupported() = default;
	};

	class VulkanDevice_FeatureUnsupported : public intern::Error
	{
	public:
		VulkanDevice_FeatureUnsupported(std::string_view name = "") : Error(std::format("Unsupported device feature.{}", name)) {}
		virtual ~VulkanDevice_FeatureUnsupported() = default;
	};
}