#include "embr/coap/header.h"

#define CASE(x) case header::x: return #x;

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

const char* header_base::to_string(classes c)
{
    switch(c)
    {
        CASE(Request)
        CASE(Success)
        CASE(ClientError)
        CASE(ServerError)
        default:    return "N/A";
    }
}

const char* header_base::to_string(codes code)
{
    switch(code)
    {
        CASE(GET)
        CASE(PUT)
        CASE(DELETE)
        CASE(POST)
        CASE(OK)
        CASE(Content)
        CASE(Created)
        default:    return "N/A";
    }
}

}

}
