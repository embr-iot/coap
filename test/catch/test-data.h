#pragma once

#include <cstdint>

#include <embr/coap/decode/breadcrumb.h>

namespace test {

namespace ids {

enum nav_data1 : int
{
    v1,
    v1_t,
    v2,
    v2_yes,
    v2_yes_id,
};

}

#define H_DATA1     0x40, 1, 0, 0
#define H_DATA2     0x42, 1, 0, 0
#define H_DATA3     0x50, 3, 0, 0
#define H_DATA4     0x43, 4, 0, 0

// Header: https://datatracker.ietf.org/doc/html/rfc7252#section-3

// In real life, GET is more likely to be NON and PUT is more likely to be CON

// GET CON header with 0 tkl, 0 mid
constexpr uint8_t h_data1[] = { H_DATA1 };
// GET CON header with 2 tkl, 0 mid
constexpr uint8_t h_data2[] = { H_DATA2 };
// PUT NON header with 0 tkl, 0 mid
constexpr uint8_t h_data3[] = { H_DATA3 };
// POST CON header with 3 tkl, 0 mid
constexpr uint8_t h_data4[] = { H_DATA4 };

// op_ = option and payload

// uri_host: "host" uri_path: "v1/t"
#define O_DATA1     0x34, 'h', 'o', 's', 't', 0x82, 'v', '1', 0x01, 't'
#define O_DATA2     0x34, 'h', 'o', 's', 't'
#define O_DATA4     0x34, 'h', 'o', 's', 't', 0x82, 'v', '2', 0x03, 'y', 'e', 's', 0x02, 'i', 'd'
#define P_DATA1     0xFF, 'h', 'e', 'l', 'l', 'o'
#define P_DATA2     0xFF, 'x'
#define P_DATA4     0xFF, 'o', 'h', 'a', 'y', 'o'

// option and payload test data
constexpr uint8_t op_data1[] = { O_DATA1, P_DATA1 };
constexpr uint8_t op_data2[] = { O_DATA2, P_DATA2 };
constexpr uint8_t op_data4[] = { O_DATA4, P_DATA4 };

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

constexpr uint8_t htop_data4[] =
{
    H_DATA4,
    3, 2, 1,    // token
    O_DATA4,    // option
    P_DATA4     // payload
};

}