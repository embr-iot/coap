#include "udp.h"

#include <embr/coap/decode.hpp>
#include <estd/span.h>

#include <esp_log.h>

static const char* TAG = "lwip-udp";

using namespace embr;

void udp_coap_recv(void *arg, 
    struct udp_pcb *pcb, struct pbuf *p,
    const ip_addr_t *addr, u16_t port)
{
    ESP_LOGI(TAG, "udp_coap_recv: entry");

    // DEBT: Bring in embr::lwip pbuf streambuf.  For now, we assume pbuf is contiguous

    assert(p->next == nullptr);
    assert(p->tot_len = p->len);
    assert(p->tot_len >= 4);

    coap::decoder<estd::ispanbuf> decoder((char*)p->payload, p->len);

    coap::header header;

    decoder >> header;

    assert(decoder.good());

    ESP_LOGI(TAG, "type=%s tkl=%u mid=%u",
        to_string(header.type()),
        header.tkl(), header.mid());

    pbuf_free(p);
}

void udp_setup()
{
    struct udp_pcb *pcb = udp_new();
    assert(pcb);

    err_t err = udp_bind(pcb, IP_ADDR_ANY, 5683);
    assert(err == ERR_OK);

    udp_recv(pcb, udp_coap_recv, NULL);    
}