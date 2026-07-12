#pragma once

#include <cstdint>

namespace test {

// GET CON header with 0 tkl, 0 mid
constexpr uint8_t h_data1[] = { 0x40, 1, 0, 0 };
constexpr uint8_t h_data2[] = { 0x42, 1, 0, 0 };

// op_ = option and payload

// option and payload test data #1
constexpr uint8_t op_data1[] = { 0x34, 'h', 'o', 's', 't', 0x82, 'v', '1', 0x01, 't', 0xFF, 'x'};

// htop_ = header, token, option and payload

constexpr uint8_t htop_data1[] =
{
    0x42, 1, 0, 0,
    1, 2,   // token
    0x34, 'h', 'o', 's', 't',       // option
    0xFF, 'x'                       // payload
};

}