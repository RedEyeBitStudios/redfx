#pragma once
#include "../base/root.hpp"
#include <cstdint>
#include <unordered_map>
#include <array>
#include "../../utils/file_binary.hpp"

namespace nxcraft
{
	using UniqID = __uint128_t;				// Unique ID
	using UPath = std::array<char, 64>;		// Unit path
}

namespace nxcraft::intern::subs
{
	class UID_Manager : public SubsystemBase
	{
	public:
		enum class UType : uint16_t
		{
			VID_WND,
			VID_WND_ACCEL
		};

		struct UID_Universal
		{
			UniqID id;
			UPath path;
			UType type;
		};

	private:
		std::unordered_map<UniqID, UID_Universal> database;
		
	public:
		UID_Manager();
		virtual ~UID_Manager();

		void reload();
		void updateBaseFD();
		void append(const UniqID id, const UID_Manager::UType type, const UPath path);
		
		std::pair<UID_Universal, FileHandleBin> queryFD(const UniqID id, const FileAccessMask mask = FileAccessMask::READ_BIT);

		constexpr UniqID makeID(std::string_view str);
		constexpr std::string makeString(const UPath& p);
	};
}