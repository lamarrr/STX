#pragma once
#include <cinttypes>

#include "stx/config.h"

STX_BEGIN_NAMESPACE

enum class [[nodiscard]] LockStatus : uint8_t
{
  Unlocked,
  Locked
};

STX_END_NAMESPACE
