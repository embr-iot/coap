#include <embr/coap/decoder.h>
#include <embr/coap/encoder.h>
#include <embr/coap/internal/constants.h>

#include <boost/asio.hpp>

#include <catch2/catch_all.hpp>

#include <estd/sstream.h>

#include <string>

using namespace embr;

namespace asio = boost::asio;

TEST_CASE("coaprd.com")
{
    // DEBT: Due to https://github.com/malachi-iot/estdlib/issues/219 we can't self interrogate for proper
    // pptr procedure, so using spanbuf
    //coap::encoder<estd::layer1::basic_out_stringbuf<char, 64, false>> out;
    uint8_t buf[128];

    coap::encoder<estd::detail::basic_ospanbuf<uint8_t>> out(buf);

    namespace ip = asio::ip;
    using udp = ip::udp;

    asio::io_context io;
    udp::socket socket(io);
    udp::resolver resolver(io);

    udp::resolver::results_type resolved = resolver.resolve(udp::v4(), "coaprd.com",
        std::to_string(coap::constants::ip::port));
    udp::endpoint endpoint = *resolved.begin();

    out << coap::header(coap::header::NON, coap::header::GET, 2, 0);
    out << coap::token{ .value{0x12, 0x34} };
    out << coap::options::uri_path << ".well-known" << "core";

    socket.open(udp::v4());
    // DEBT: Using pos() directly not ideal
    socket.send_to(asio::buffer(buf, out.out().pos()), endpoint);

    udp::endpoint sender;
    std::size_t n = socket.receive_from(asio::buffer(buf), sender);

    REQUIRE(n > 4);
    REQUIRE(n <= sizeof(buf));

    coap::decoder<estd::detail::basic_ispanbuf<uint8_t>> in(buf, n);

    coap::header header;
    coap::token token;
    coap::options::option<> option;
    using numbers = coap::options::numbers;

    in >> header;

    REQUIRE(in.good());

    REQUIRE(header.valid());
    REQUIRE(header.tkl() == 2);
    REQUIRE(header.type() == coap::header::NON);

    in >> token;

    REQUIRE(token.value[0] == 0x12);
    REQUIRE(token.value[1] == 0x34);
    REQUIRE(token.size == 2);
    //REQUIRE(estd::span(token.value, token.size) == estd::span())

    in >> option;

    REQUIRE(option.number == numbers::ContentFormat);
    REQUIRE(option.uint() == coap::options::content_formats::ApplicationLinkFormat);

    /*
    do
    {
        in >> option;

        REQUIRE(in.good());
    }
    while(in.state() == in.Options); */

    // Nifty idea, but interrogating state is cleaner.  Otherwise what do we do?  Set bad bit? assert?
    //in >> coap::payload;

    REQUIRE(in.state() == in.Payload);

    int len = in.in().egptr() - in.in().gptr();

    std::string_view payload((char*)in.in().gptr(), len);

    // DEBT: Fragile - but we don't have a link format parser just yet
    int pos = payload.find_first_of("</.well-known/core>; ct=40");

    REQUIRE(pos != std::string_view::npos);
}
