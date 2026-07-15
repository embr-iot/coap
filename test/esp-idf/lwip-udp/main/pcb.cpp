#include "udp.h"

void udp_coap_recv(void *arg, 
    struct udp_pcb *pcb, struct pbuf *p,
    const ip_addr_t *addr, u16_t port)
{
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