#pragma once
#include <cstdint>
#include <core/mirage.h>
#include <memory>
#include <boost/asio/buffer.hpp>
#include <type_traits>
#include <boost/asio/placeholders.hpp>
#include "packet.h"

template<> struct fmt::formatter<mirage::network::PacketId> 
	: mirage::utils::EnumFormatter<mirage::network::PacketId> {};

namespace mirage::network
{
	template<typename T>
	inline T fromString(const std::string& sv, unsigned short port)
	{
		boost::system::error_code ec;
		boost::asio::ip::address address = 
			boost::asio::ip::address::from_string(sv, ec);
		if(ec.value() != 0)
		{
			// FIXME:
			abort();
		}

		return T{address, port};
	}

	inline auto fromStringUdp(const std::string& sv, unsigned short port)
	{
		return fromString<boost::asio::ip::udp::endpoint>(sv, port);
	}

	inline auto fromStringTcp(const std::string& sv, unsigned short port)
	{
		return fromString<boost::asio::ip::tcp::endpoint>(sv, port);
	}

	// a view to a packet
	struct AbstractPacket
	{
		const PacketVoid* packet = nullptr;
		size_t size = 0;

		template<typename T>
		AbstractPacket(T& packet_)
		{
			packet = reinterpret_cast<const PacketVoid*>(&packet_);
			size = sizeof(std::decay_t<T>);
		}
		AbstractPacket(const PacketVoid* const packet_, size_t size_)
			: packet{packet_}, size{size_} {}
	
		AbstractPacket(const AbstractPacket& other);
		AbstractPacket(AbstractPacket&& other) noexcept;

		AbstractPacket& operator=(const AbstractPacket& other);	
		AbstractPacket& operator=(AbstractPacket&& other) noexcept;	
		operator boost::asio::const_buffer(void) const;	
	};

	template<typename T>
	inline const T packetCast(const AbstractPacket& packet)
	{
		if(!packet.packet)
			; // FIXME:
		T npacket;
		memcpy(static_cast<void*>(&npacket), static_cast<const void*>(packet.packet), sizeof(T));
		return npacket;
	}
}

inline mirage::network::AbstractPacket::AbstractPacket(
		const mirage::network::AbstractPacket& other)
{
	(*this) = other;
}

inline mirage::network::AbstractPacket::AbstractPacket(
		mirage::network::AbstractPacket&& other) noexcept
{
	(*this) = other;
}

inline mirage::network::AbstractPacket::operator boost::asio::const_buffer(void) const
{
	return boost::asio::const_buffer(packet, size);
}

inline mirage::network::AbstractPacket& 
	mirage::network::AbstractPacket::operator=(const AbstractPacket& other)
{
	if(this == &other)
		return *this;
	packet = other.packet;
	size = other.size;
	return *this;
}

inline mirage::network::AbstractPacket& 
	mirage::network::AbstractPacket::operator=(AbstractPacket&& other) noexcept
{
	packet = std::move(other.packet);
	size = other.size;
	return *this;
}
