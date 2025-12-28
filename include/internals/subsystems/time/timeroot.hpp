#pragma once
#include "../../bases/subsystem.hpp"
#include <unordered_map>
#include <chrono>
#include <string_view>
#include <unordered_set>

namespace nxcraft::intern::subsystems
{
	class TimeRoot : public SubsystemBase
	{
	public:
		class Registry
		{
		private:
			std::unordered_map
			<
				std::string,
				std::chrono::milliseconds
			> data;
		public:
			Registry();
			~Registry() = default;

			std::chrono::milliseconds& operator[](std::string_view entry_name);
		};

		class Point
		{
		private:
			std::chrono::system_clock::time_point first_time_point;
			std::string_view entry_name;
		public:
			Point(std::string_view entry_name);
			~Point();
		};

		std::unique_ptr<Registry> component_registry;
	public:
		TimeRoot();
		~TimeRoot();

		Registry& getComponentRegistry();
	};
}

namespace nxcraft
{
	using TimePoint = nxcraft::intern::subsystems::TimeRoot::Point;
	static const char* delta_time_entry_name = "delta";
}