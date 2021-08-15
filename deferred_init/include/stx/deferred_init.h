
#pragma once

#include "stx/config.h"

STX_BEGIN_NAMESPACE

struct deferred_init_tag {};

constexpr const deferred_init_tag deferred_init{};

STX_END_NAMESPACE
