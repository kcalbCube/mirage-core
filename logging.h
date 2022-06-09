#pragma once

#define FMT_HEADER_ONLY
#define FMTLOG_HEADER_ONLY

#include <fmt/core.h>
#include <fmt/chrono.h>

#define FMT_NOEXCEPT noexcept
#include <fmtlog/fmtlog.h>

using namespace std::literals::chrono_literals;
