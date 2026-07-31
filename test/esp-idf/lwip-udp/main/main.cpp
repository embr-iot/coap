#include <stdio.h>

#include <embr/wifi/fwd.h>
#include <embr/esp-idf/wifi/fwd.h>

void udp_setup();

using namespace embr;

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(simple_flash_init());
    ESP_ERROR_CHECK(wifi::simple_init());

    udp_setup();
}
