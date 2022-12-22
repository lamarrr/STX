#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include "stx/config.h"

STX_BEGIN_NAMESPACE

constexpr auto u8_min = std::numeric_limits<uint8_t>::min();
constexpr auto u8_max = std::numeric_limits<uint8_t>::max();
constexpr auto u16_min = std::numeric_limits<uint16_t>::min();
constexpr auto u16_max = std::numeric_limits<uint16_t>::max();
constexpr auto u32_min = std::numeric_limits<uint32_t>::min();
constexpr auto u32_max = std::numeric_limits<uint32_t>::max();
constexpr auto u64_min = std::numeric_limits<uint64_t>::min();
constexpr auto u64_max = std::numeric_limits<uint64_t>::max();
constexpr auto usize_min = std::numeric_limits<size_t>::min();
constexpr auto usize_max = std::numeric_limits<size_t>::max();

constexpr auto i8_min = std::numeric_limits<int8_t>::min();
constexpr auto i8_max = std::numeric_limits<int8_t>::max();
constexpr auto i16_min = std::numeric_limits<int16_t>::min();
constexpr auto i16_max = std::numeric_limits<int16_t>::max();
constexpr auto i32_min = std::numeric_limits<int32_t>::min();
constexpr auto i32_max = std::numeric_limits<int32_t>::max();
constexpr auto i64_min = std::numeric_limits<int64_t>::min();
constexpr auto i64_max = std::numeric_limits<int64_t>::max();

constexpr auto f32_min = std::numeric_limits<float>::min();
constexpr auto f32_max = std::numeric_limits<float>::max();
constexpr auto f64_min = std::numeric_limits<double>::min();
constexpr auto f64_max = std::numeric_limits<double>::max();

constexpr auto f32_epsilon = std::numeric_limits<float>::epsilon();
constexpr auto f64_epsilon = std::numeric_limits<double>::epsilon();

STX_END_NAMESPACE
