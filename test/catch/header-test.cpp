#include "test-data.h"

#include <embr/coap/header.h>

#include <catch2/catch_all.hpp>

using namespace embr;

TEST_CASE("header")
{
    using codes = coap::header;

    coap::header h1{};

    h1.tkl(1);
    h1.type(coap::header::NON);
    h1.code(coap::header::PUT);
    h1.mid(1);

    constexpr coap::header h2{codes::ACK, codes::Content};
    constexpr coap::header nullish{codes::CON, codes::GET};

    REQUIRE(h1.tkl() == 1);

    auto h3 = (const coap::header*) test::h_data1;

    REQUIRE(*h3 == nullish);
}
