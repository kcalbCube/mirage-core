#include "network.h"

auto mirage::network::fromStringUdp(const std::string& sv, unsigned short port)
{
	return fromString<boost::asio::ip::udp::endpoint>(sv, port);
}

auto mirage::network::fromStringTcp(const std::string& sv, unsigned short port)
{
	return fromString<boost::asio::ip::tcp::endpoint>(sv, port);
}

mirage::network::AbstractPacket::AbstractPacket(
		const mirage::network::AbstractPacket& other)
{
	(*this) = other;
}

mirage::network::AbstractPacket::AbstractPacket(
		mirage::network::AbstractPacket&& other) noexcept
{
	(*this) = other;
}

mirage::network::AbstractPacket::operator boost::asio::const_buffer(void) const
{
	return boost::asio::const_buffer(packet, size);
}

mirage::network::AbstractPacket& 
	mirage::network::AbstractPacket::operator=(const AbstractPacket& other)
{
	if(this == &other)
		return *this;
	packet = other.packet;
	size = other.size;
	return *this;
}

mirage::network::AbstractPacket& 
	mirage::network::AbstractPacket::operator=(AbstractPacket&& other) noexcept
{
	packet = std::move(other.packet);
	size = other.size;
	return *this;
}
