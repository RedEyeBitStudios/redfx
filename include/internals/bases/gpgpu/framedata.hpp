#pragma once
#include "pass_pool.hpp"
#include <optional>

namespace nxcraft::intern::subsystems
{
	class FrameData
	{
	public:
		GPGPU_PassPool pass_resources;
		
		FrameData() = default;
		virtual ~FrameData() = default;
	};
}