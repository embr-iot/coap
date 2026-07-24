#pragma once

#include <estd/cstdint.h>

namespace embr::coap::internal {

#define EMBR_COAP_RESPONSE_CODE(class, detail)  ( class << 5 | detail )

struct header_base
{
    enum classes : uint8_t
    {
        Request,
        Success = 2,
        ClientError = 4,
        ServerError
    };

    // Deviating PascalCase convention to better align with https://datatracker.ietf.org/doc/html/rfc7252
    enum codes : uint8_t
    {
        GET = 1,
        POST,
        PUT,
        DELETE,

        OK                      = EMBR_COAP_RESPONSE_CODE(Success, 00),
        Created                 = EMBR_COAP_RESPONSE_CODE(Success, 01),
        Content                 = EMBR_COAP_RESPONSE_CODE(Success, 05),
        NotFound                = EMBR_COAP_RESPONSE_CODE(Success, 04),
        UnsupportedContentFormat    = EMBR_COAP_RESPONSE_CODE(ClientError, 15),
        InternalServerError     = EMBR_COAP_RESPONSE_CODE(ServerError, 00),
    };

    static const char* to_string(classes);
    static const char* to_string(codes);
};


constexpr header_base::classes get_class(header_base::codes c)
{
    return static_cast<header_base::classes>(c >> 5);
}

inline const char* to_string(header_base::classes c) { return header_base::to_string(c); }
inline const char* to_string(header_base::codes c) { return header_base::to_string(c); }

}
