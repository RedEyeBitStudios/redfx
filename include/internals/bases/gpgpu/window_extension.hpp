#pragma once

namespace nxcraft::intern::subsystems
{
	class GPGPU_WindowExtension
	{
	public:
		GPGPU_WindowExtension(GPGPU_WindowExtension&) = delete;
		GPGPU_WindowExtension(GPGPU_WindowExtension&&) = delete;

		GPGPU_WindowExtension() = default;
		virtual ~GPGPU_WindowExtension() = default;
	};
}