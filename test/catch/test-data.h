#pragma once

#include <cstdint>

namespace test {

#define H_DATA1     0x40, 1, 0, 0
#define H_DATA2     0x42, 1, 0, 0
#define H_DATA3     0x50, 3, 0, 0

// GET CON header with 0 tkl, 0 mid
constexpr uint8_t h_data1[] = { 0x40, 1, 0, 0 };
// GET CON header with 2 tkl, 0 mid
constexpr uint8_t h_data2[] = { H_DATA2 };
constexpr uint8_t h_data3[] = { H_DATA3 };

// op_ = option and payload

// uri_host: "host" uri_path: "v1/t"
#define O_DATA1     0x34, 'h', 'o', 's', 't', 0x82, 'v', '1', 0x01, 't'
#define O_DATA2     0x34, 'h', 'o', 's', 't'
#define P_DATA1     0xFF, 'h', 'e', 'l', 'l', 'o'
#define P_DATA2     0xFF, 'x'

// option and payload test data #1
constexpr uint8_t op_data1[] = { O_DATA1, P_DATA1 };
constexpr uint8_t op_data2[] = { O_DATA2, P_DATA2 };

// htop_ = header, token, option and payload

constexpr uint8_t htop_data1[] =
{
    H_DATA1,
    O_DATA1,    // option
    P_DATA1     // payload
};

constexpr uint8_t htop_data2[] =
{
    H_DATA2,
    1, 2,       // token
    O_DATA2,    // option
    P_DATA2     // payload
};

}