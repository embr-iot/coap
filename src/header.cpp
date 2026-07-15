#include "embr/coap/header.h"

namespace embr::coap {

const char* to_string(header::types type)
{
    switch(type)
    {
        case header::NON:   return "NON";
        case header::CON:   return "CON";
        case header::ACK:   return "ACK";
        case header::RST:   return "RST";
    }

    abort();
}

namespace internal {

const char* header_base::to_string(request_codes code)
{
    switch(code)
    {
        default:    return "N/A";
    }
}

}

}
