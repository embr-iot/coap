#include <embr/coap/uint.h>

#include <catch2/catch_all.hpp>

using namespace embr;

TEST_CASE("uint")
{
    uint8_t buf[16];

    // DEBT: Naming is intermingling of hex and dec, fix that
    constexpr uint8_t in_16[]       { 0x10 };
    constexpr uint8_t in_0001[]     { 0,    1 };
    constexpr uint8_t in_0016[]     { 0, 0x10 };
    constexpr uint8_t in_0017[]     { 0, 0x11 };
    constexpr uint8_t in_1234[]     { 4, 0xD2 };
    constexpr uint8_t in_123456[]   { 1, 0xE2, 0x40 };
    constexpr uint8_t in_000002[]   { 0,    0,    2 };

    SECTION("encode")
    {
        SECTION("fixed length")
        {
            coap::uint_encode_fixed(buf, buf + 2, 0x1234);

            REQUIRE(buf[0] == 0x12);
            REQUIRE(buf[1] == 0x34);
        }
        SECTION("deduced length")
        {
            uint8_t* end;

            end = coap::uint_encode(buf, buf + sizeof(buf), 0x80);

            REQUIRE(end - buf == 1);

            end = coap::uint_encode(buf, buf + sizeof(buf), 0x123456);

            REQUIRE(end - buf == 3);

            REQUIRE(buf[0] == 0x12);
            REQUIRE(buf[1] == 0x34);
            REQUIRE(buf[2] == 0x56);
        }
    }
    SECTION("decode")
    {
        auto v = coap::uint_decode<unsigned>(in_16);

        REQUIRE(v == 16);

        v = coap::uint_decode<unsigned>(in_0001);

        REQUIRE(v == 1);

        v = coap::uint_decode<unsigned>(in_1234);

        REQUIRE(v == 1234);

        v = coap::uint_decode<unsigned>(in_123456);

        REQUIRE(v == 123456);
    }
    SECTION("bigint")
    {
        int r = be_uintcmp(in_16, in_0001);

        REQUIRE(r > 0);

        r = be_uintcmp(in_16, in_0016);

        REQUIRE(r == 0);

        r = be_uintcmp(in_16, in_0017);

        REQUIRE(r < 0);

        r = be_uintcmp(in_0001, in_16);

        REQUIRE(r < 0);

        r = be_uintcmp(in_0016, in_16);

        REQUIRE(r == 0);

        r = be_uintcmp(in_0017, in_16);

        REQUIRE(r > 0);

        r = be_uintcmp(in_000002, in_16);

        REQUIRE(r < 0);
    }
    SECTION("uint standalone")
    {
        coap::const_uint v2(in_123456, 3);
        coap::uint v3(buf);

        REQUIRE(v2 == 123456);

        v3 = 1234;

        REQUIRE(v3 == coap::const_uint(in_1234));
        REQUIRE(v3.compare(v2) < 0);
    }
}
