#include "utility.h"

template<typename Func>
std::string mirage::utils::sanitize(std::string str, Func&& filter)
{
	return boost::remove_erase_if(str, !boost::bind(filter, _1));
}

std::string mirage::utils::sanitizeUsername(std::string str)
{
	return sanitize(std::move(str), isalnum);
}

std::string_view mirage::utils::stringView(const char* const src, const size_t maxSize)
{
	if(!src)
		return "";
	const auto* const zeroPos = 
		static_cast<const char* const>(memchr(src, '\0', maxSize));
	if(!zeroPos)
		return "";

	const auto size = static_cast<size_t>(zeroPos - src);

	if(!size)
		return "";

	return std::string_view(src, size);
}