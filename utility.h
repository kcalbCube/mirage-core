#pragma once
#include <sstream>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#include <algorithm>
#include <string>
#include <boost/range/algorithm_ext/erase.hpp>
#include "static.h"
#include "logging.h"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>	

using namespace boost::placeholders;
namespace mirage::utils
{
	template<typename Func>
	std::string sanitize(std::string str, Func&& filter);	
	
	std::string sanitizeUsername(std::string str);	

	/*
	 * safe stringView for memarrays.
	 * returns "" if string invalid, if valid returned 
	 * string_view size will be equal to strlen(src)
	 */
	std::string_view stringView(const char* const src, const size_t maxSize);

	template<typename Array>
	std::string_view stringView(Array& array);

	template<typename Array>
	std::string_view span(Array& array);

	constexpr auto serializationFlags = boost::archive::no_header | boost::archive::no_tracking;

	std::string serialize(const auto& value);	

	template<typename T>
	T deserialize(const std::string& sv);	

	template<typename T>
	struct Vec3
	{
		T 
			x{},
			y{},
			z{};

		void serialize(auto& ar, const unsigned);	
	};
}

template<typename Array>
std::string_view mirage::utils::stringView(Array& array)
{
	return stringView(array, std::size(array));
}

template<typename Array>
std::string_view mirage::utils::span(Array& array)
{
	return std::string_view(array, std::size(array));
}

template<typename T> 
struct std::less<std::unique_ptr<T>> 
{
	bool operator()(const std::unique_ptr<T>& a, const std::unique_ptr<T>& b) const 
	{
		return *a < *b;
	}
};

inline std::string mirage::utils::serialize(const auto& value)
{
	std::stringstream ss{};
	boost::archive::binary_oarchive archive{ss, serializationFlags};
	
	archive << value;

	return ss.str();
}

template<typename T>
inline T mirage::utils::deserialize(const std::string& sv)
{
	std::istringstream ss{sv};	

	boost::archive::binary_iarchive archive{ss, serializationFlags};
	
	T value;

	archive >> value;

	return value;
}

template<typename T>
inline void mirage::utils::Vec3<T>::serialize(auto& ar, const unsigned)
{
	ar & x;
	ar & y;
	ar & z;
}
