#include "udp.h"

#include <embr/coap/encode.h>
#include <embr/coap/decode.hpp>
#include <embr/lwip/shared_pbuf.h>

#include <embr/platform/lwip/streambuf.h>

#include <estd/span.h>

#include <esp_log.h>

static const char* TAG = "lwip-udp";

using namespace embr;

/* failed experiment, these are abiguous
static void test(pbuf* p)
{

}

static void test(pbuf*&& p)
{

}

void test3()
{
    pbuf backing;
    pbuf* dummy = &backing;

    test(dummy);
    test(std::move(dummy));
}
*/

void udp_coap_recv(void* arg, 
    udp_pcb* pcb, pbuf* p,
    const ip_addr_t* addr, u16_t port)
{
    ESP_LOGI(TAG, "udp_coap_recv: entry");

    using streambuf_type = lwip::ipbuf_streambuf;

    lwip::shared_pbuf owned = take_ownership(p);

    // NOTE: ipbuf_streambuf ownsership of pbuf is awkward,
    // while opbuf_streambuf feels natural.  See if we can displace the
    // too-magic (pbuf, bool) ownership signature with something more
    // explicit like 'take_ownership(pbuf)' 
    coap::decoder<streambuf_type> decoder(owned);

    coap::header header;

    decoder >> header;

    assert(decoder.good());

    ESP_LOGI(TAG, "type=%s tkl=%u mid=%u",
        to_string(header.type()),
        header.tkl(), header.mid());

    coap::encoder<lwip::opbuf_streambuf> encoder(16);

    header.type(header.ACK);
    header.code(header.OK);

    encoder << header;

    pbuf* out = encoder.out().pbuf();

    // DEBT: Really we expect this to happen in opbuf_streambuf, don't we?  Or
    // do we use that shrink call?
    out->len = 4;
    out->tot_len = 4;

    udp_sendto(pcb, out, addr, port);
}

void udp_setup()
{
    udp_pcb* pcb = udp_new();
    assert(pcb);

    err_t err = udp_bind(pcb, IP_ADDR_ANY, 5683);
    assert(err == ERR_OK);

    udp_recv(pcb, udp_coap_recv, NULL);    
}