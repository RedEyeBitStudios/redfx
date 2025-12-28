#pragma once
#include <memory>

namespace nxcraft::intern::subsystems
{
	class SubsystemBase
	{
	public:
		SubsystemBase() = default;
		virtual ~SubsystemBase() = default;
	};
}