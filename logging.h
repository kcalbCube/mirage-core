#pragma once

#define FMT_NOEXCEPT noexcept
#include <fmt/core.h>
#include <fmt/chrono.h>

#include <fmtlog/fmtlog.h>
#include <magic_enum.hpp>

using namespace std::literals::chrono_literals;

// Use outside namespaces
#define MIRAGE_ENUM_FORMATTER(T) template<> struct fmt::formatter<T> : mirage::utils::EnumFormatter<T> {}

namespace mirage::utils
{
	template<typename T>
	struct EnumFormatter : fmt::formatter<std::string_view>
	{
		template<typename FormatContext>
		auto format(T en, FormatContext& ctx)
		{
			return fmt::formatter<std::string_view>::format(magic_enum::enum_name(en), ctx);
		}
	};
}