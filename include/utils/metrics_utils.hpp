#pragma once
#include <stdfloat>
#include <cstdint>
#include <string>
#include <format>
#include <array>
#include <initializer_list>

/*
namespace nxcraft::intern::utils
{
	template<uint64_t bytes_per_one, std::array<char, 3> extension>
	class ByteInterface
	{
	protected:
		uint64_t bytes_per_one;
		uint64_t value;
		std::array<char, 3> extension;
	public:
		ByteInterface(const uint64_t value, const uint64_t bytes_per_one, const std::array<char, 3> extension)
		{
			this->value = value;
			this->bytes_per_one = bytes_per_one;
			
			this->extension = extension;
		}
		virtual ~ByteInterface() = default;

		std::string getString() const
		{
			return std::format("{} {}", this->value, this->extension)
		}
		uint64_t toBytes() const
		{
			return this->value * this->bytes_per_one;
		}
	};
}

namespace nxcraft
{
	class Byte : public intern::utils::ByteInterface<1, >
	{
	public:
		Byte(const uint64_t value) : ByteInterface(value, 1, {"B"})
		{
		}
		virtual ~Byte() = default;
	};

	class KiloByte : public intern::utils::ByteInterface<1024>
	{
	public:
		KiloByte(const uint64_t value) : ByteInterface(value)
		{
			this->extension = "KB";
		}
		KiloByte(Byte&& b) : ByteInterface(0)
		{
			this->value = b.toBytes() / this->bytes_per_one;
		}
		virtual ~KiloByte() = default;
	};

	class MegaByte : public intern::utils::ByteInterface<1024 * 1024>
	{
	public:
		MegaByte(const uint64_t value) : ByteInterface(value)
		{
			this->extension = "MB";
		}
		MegaByte(Byte&& b) : ByteInterface(0)
		{
			this->value = b.toBytes() / this->bytes_per_one;
		}
		MegaByte(KiloByte&& b) : ByteInterface(0)
		{
			this->value = b.toBytes() / this->bytes_per_one;
		}
		virtual ~MegaByte() = default;
	};

	class GigaByte : public intern::utils::ByteInterface<1024 * 1024 * 1024>
	{
	public:
		GigaByte(const uint64_t value) : ByteInterface(value)
		{
			this->extension = "GB";
		}
		GigaByte(Byte&& b) : ByteInterface(0)
		{
			this->value = b.toBytes() / this->bytes_per_one;
		}
		GigaByte(KiloByte&& b) : ByteInterface(0)
		{
			this->value = b.toBytes() / this->bytes_per_one;
		}
		GigaByte(MegaByte&& b) : ByteInterface(0)
		{
			this->value = b.toBytes() / this->bytes_per_one;
		}
		virtual ~GigaByte() = default;
	};


}
*/