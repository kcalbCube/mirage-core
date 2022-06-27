#pragma once
#include <cstddef>
#include "static.h"
#include <boost/asio.hpp>
#include "event.h"
namespace mirage
{
	constexpr int version = 1;	
	MIRAGE_COFU(boost::asio::io_context, ioContext);

	class Tickrate
	{
		uint8_t frequency = 5;
	public:

		uint8_t getFrequncy(void) const noexcept
		{
			return frequency;
		}

		uint16_t getPeriod(void) const noexcept
		{
			return 1000 / static_cast<uint16_t>(frequency);
		}

		void setFrequency(uint8_t newFrequency) noexcept;	

		void setPeriod(uint16_t newPeriod) noexcept
		{
			setFrequency(1000 / newPeriod);
		}
	};

	struct TickrateChanged
	{
		Tickrate old;
	};

	MIRAGE_COFU(Tickrate, tickrate);
}

inline void mirage::Tickrate::setFrequency(uint8_t newFrequency) noexcept
{
	auto copy = *this;
	frequency = newFrequency;
	event::triggerEvent<TickrateChanged>(std::move(copy));
}