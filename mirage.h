#pragma once
#include <cstddef>
#include "utility.h"
#include <boost/asio.hpp>
#include "processing.h"

namespace mirage
{
	constexpr int version = 1;	
	MIRAGE_COFU(boost::asio::io_context, ioContext);
}
