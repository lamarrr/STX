#pragma once

#define STX_TRY__UTIL_JOIN_(x, y) x##_##y
#define STX_WITH_UNIQUE_SUFFIX_(x, y) STX_TRY__UTIL_JOIN_(x, y)
