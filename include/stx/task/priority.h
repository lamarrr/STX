#pragma once

#include "stx/config.h"
#include "stx/limits.h"

STX_BEGIN_NAMESPACE

/// Some tasks can take an unreasonably long time to complete. and we can't wait
/// for them to complete, we therefore need these priorities along with a
/// cancelation mechanism.
/// this will enable us make smart decisions about graceful shutdown and task
/// prioritization.
///
///
/// these are just hints to the executors and they don't need to support or
/// respond to them.
enum class TaskPriority : uint8_t
{
};

constexpr TaskPriority NORMAL_PRIORITY{0};

// involves tasks that the user needs to
// observe its result as soon as possible. i.e. image loading and decoding,
// texture loading, offscreen rendering, etc.
//
constexpr TaskPriority INTERACTIVE_PRIORITY{1};

// critical tasks can involve tasks saving user data. i.e. backing up user
// data, saving changes to disk, etc.
//
constexpr TaskPriority CRITICAL_PRIORITY{U8_MAX};

STX_END_NAMESPACE
