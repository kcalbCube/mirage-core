#pragma once
#include "utility.h"

namespace mirage
{
	struct ClientInformation
	{
		uint16_t screenWidth,
			 screenHeight;

		void serialize(auto& ar, const unsigned)
		{
			ar & screenWidth;
			ar & screenHeight;
		}
	};
}
