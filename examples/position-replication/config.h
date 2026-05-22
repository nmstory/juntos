#pragma once
#include <cstdint>

constexpr uint32_t CLIENT_BITS = 8;
constexpr uint32_t LOCAL_BITS  = 32 - CLIENT_BITS;

constexpr uint32_t LOCAL_MASK  = (1u << LOCAL_BITS) - 1;
constexpr uint32_t CLIENT_MASK = (1u << CLIENT_BITS) - 1;