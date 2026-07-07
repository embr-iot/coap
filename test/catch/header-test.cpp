#include <embr/coap/header.h>

#include <catch2/catch_all.hpp>

using namespace embr;

TEST_CASE("header")
{
    using codes = coap::header;

    coap::header h1{};

    h1.type(coap::header::NON);
    h1.code(coap::header::PUT);
    h1.mid(1);

    constexpr coap::header h2{codes::ACK, codes::Content, 0};
}
