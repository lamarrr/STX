#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include "stx/config.h"

STX_BEGIN_NAMESPACE

constexpr auto U8_MIN = std::numeric_limits<uint8_t>::min();
constexpr auto U8_MAX = std::numeric_limits<uint8_t>::max();
constexpr auto U16_MIN = std::numeric_limits<uint16_t>::min();
constexpr auto U16_MAX = std::numeric_limits<uint16_t>::max();
constexpr auto U32_MIN = std::numeric_limits<uint32_t>::min();
constexpr auto U32_MAX = std::numeric_limits<uint32_t>::max();
constexpr auto U64_MIN = std::numeric_limits<uint64_t>::min();
constexpr auto U64_MAX = std::numeric_limits<uint64_t>::max();
constexpr auto USIZE_MIN = std::numeric_limits<size_t>::min();
constexpr auto USIZE_MAX = std::numeric_limits<size_t>::max();

constexpr auto I8_MIN = std::numeric_limits<int8_t>::min();
constexpr auto I8_MAX = std::numeric_limits<int8_t>::max();
constexpr auto I16_MIN = std::numeric_limits<int16_t>::min();
constexpr auto I16_MAX = std::numeric_limits<int16_t>::max();
constexpr auto I32_MIN = std::numeric_limits<int32_t>::min();
constexpr auto I32_MAX = std::numeric_limits<int32_t>::max();
constexpr auto I64_MIN = std::numeric_limits<int64_t>::min();
constexpr auto I64_MAX = std::numeric_limits<int64_t>::max();

constexpr auto U_MIN = std::numeric_limits<unsigned int>::min();
constexpr auto U_MAX = std::numeric_limits<unsigned int>::max();

constexpr auto I_MIN = std::numeric_limits<int>::min();
constexpr auto I_MAX = std::numeric_limits<int>::max();

constexpr auto F32_MIN_POSITIVE = std::numeric_limits<float>::min();
constexpr auto F32_MIN = -std::numeric_limits<float>::max();
constexpr auto F32_MAX = std::numeric_limits<float>::max();

constexpr auto F64_MIN_POSITIVE = std::numeric_limits<double>::min();
constexpr auto F64_MIN = -std::numeric_limits<double>::max();
constexpr auto F64_MAX = std::numeric_limits<double>::max();

constexpr auto F32_EPSILON = std::numeric_limits<float>::epsilon();
constexpr auto F64_EPSILON = std::numeric_limits<double>::epsilon();

STX_END_NAMESPACE
