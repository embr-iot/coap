#include "embr/coap/header.h"

// 24SEP26 MB DEBT: Move these out to to_string

#define CASE(x) case header::x: return #x;

namespace embr::coap {

namespace internal {

const char* to_string(header_base::types type)
{
    using h = header_base;

    switch(type)
    {
        case h::NON:   return "NON";
        case h::CON:   return "CON";
        case h::ACK:   return "ACK";
        case h::RST:   return "RST";
    }

    abort();
}

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
