#pragma once
#include "../uid/uid_manager.hpp"

namespace nxcraft::intern::subs
{
	class AccelRoot : public SubsystemBase
	{
	public:
		AccelRoot();
		virtual ~AccelRoot() = default;

		virtual void registerWnd(const UniqID& id) = 0;
	};
}