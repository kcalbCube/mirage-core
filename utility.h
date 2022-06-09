#pragma once
#include "boost/archive/text_oarchive.hpp"
#include <sstream>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#include <algorithm>
#include <string>
#include <boost/range/algorithm_ext/erase.hpp>
#include "static.h"
#include <magic_enum.hpp>
#include "logging.h"
#include <boost/uuid/uuid.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/variant.hpp>
#define MIRAGE_COFU(T, name, ...) \
	inline struct _##name##cofu { T* instance = nullptr; T& operator()(void) { if(!instance) instance = new T(__VA_ARGS__); \
	return *instance; }; static bool destructed; ~_##name##cofu(void) { destructed = true; if(instance) delete instance;} \
	static bool isDestructed(void) { return destructed; } } name; inline bool _##name##cofu::destructed = false
	
using namespace boost::placeholders;
namespace mirage::utils
{
	inline std::string sanitize(std::string str, std::function<bool(char)> filter)
	{
		return boost::remove_erase_if(str, !boost::bind(filter, _1));
	}
	
	inline std::string sanitizeUsername(std::string_view str)
	{
		return sanitize(std::string{str}, isalnum);
	}
/*
 *
 * @example: template<> struct fmt::formatter<MyEnum> : EnumFormatter<MyEnum> {};
 */
	template<typename T>
	struct EnumFormatter : fmt::formatter<std::string_view>
	{
		template<typename FormatContext>
		auto format(T en, FormatContext& ctx)
		{
			return fmt::formatter<std::string_view>::format(magic_enum::enum_name(en), ctx);
		}
	};

	/*
	 * safe stringView for memarrays.
	 * returns "" if string invalid, if valid returned 
	 * string_view size will be equal to strlen(src)
	 */
	inline std::string_view stringView(const char* const src, const size_t maxSize)
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

	inline std::string serialize(const auto& value)
	{
		std::stringstream ss{};
		boost::archive::text_oarchive archive{ss};
		
		archive << value;

		return ss.str();
	}

	template<typename T>
	inline T deserialize(const std::string& sv)
	{
		std::istringstream ss{sv};	

		boost::archive::text_iarchive archive{ss};
		
		T value;

		archive >> value;

		return value;
	}

	template<typename T>
	struct Vec3
	{
		T 
			x{},
			y{},
			z{};

		inline void serialize(auto& ar, const unsigned)
		{
			ar & x;
			ar & y;
			ar & z;
		};
	};
}
