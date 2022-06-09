#pragma once
#include <string>
#include <boost/uuid/uuid.hpp>
#include "utility.h"
#include "graphics.h"
#include <boost/asio/ip/udp.hpp>

namespace mirage::network
{
	constexpr uint32_t packetConstant = 0x4752494D;
	constexpr size_t maxPacketSize = 1024;
	constexpr size_t usernameMax = 32;

	enum class PacketId : uint8_t
	{
		zero,
		timeout,
		/* 
		 * c->s InitializeConnection
		 * s->c ConnectionResponce
		 */
		connect,
		/* 
		 * c->s ClientDisconnect
		 * s->c ServerDisconnect
		 */
		disconnect,
		/* 
		 * MessageSent
		 */
		message,
		/*
		 * s->c ServerInfo
		 */
		serverInfo,
		/*
		 * s->c GraphicFrame
		 */
		graphicFrame,
		/*
		 * s->c ResourceUpdate
		 * c->s ResourceRequest
		 */
		resource
	};

#pragma pack(push, 1)
	template<PacketId ID>
	struct Packet
	{
		uint32_t constant = packetConstant;
		PacketId id = ID;
	};

	using PacketVoid = Packet<PacketId::zero>;

	struct Timeout : Packet<PacketId::timeout>
	{
	};

	struct InitializeConnection : Packet<PacketId::connect>
	{
		char username[usernameMax]{};
	};

	struct ConnectionResponce : Packet<PacketId::connect>
	{
		enum : uint8_t
		{
			unavailable,
			banned,
			alreadyConnected,
			success,
		} responce;
	};

	struct ClientDisconnect : Packet<PacketId::disconnect>
	{
	};

	struct ServerDisconnect : Packet<PacketId::disconnect>
	{
	};
	
	struct MessageSent : Packet<PacketId::message>
	{
		static constexpr size_t messageMax = 128;
		char message[messageMax]{};

		std::string_view view(void) const
		{
			return utils::stringView(message, messageMax);
		}
	};

	struct ServerInfo : Packet<PacketId::serverInfo>
	{
		static constexpr size_t infoMax = 512;
		char info[infoMax]{}; // json
	};

	struct GraphicFrame : Packet<PacketId::graphicFrame>
	{
		using SerializedT = std::vector<graphics::VerticeGroup>;
		char serialized[256];	
	};

	struct ResourceUpdate : Packet<PacketId::resource>
	{	
		using SerializedT = std::vector<graphics::IconResource>;
		char serialized[256];
	};

	struct ResourceRequest : Packet<PacketId::resource>
	{
		using SerializedT = std::vector<graphics::Icon>;
		char serialized[256];
	};

#pragma pack(pop)

	struct AbstractPacket;
}
