#pragma once

#include <estd/cstdint.h>

namespace embr::coap::internal {

// See https://datatracker.ietf.org/doc/html/rfc7252#section-12.1.1

#define EMBR_COAP_RESPONSE_CODE(class, detail)  ( class << 5U | unsigned(detail) )

struct header_base
{
    enum classes : unsigned
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

        OK                      = EMBR_COAP_RESPONSE_CODE(Success, 0),
        Created                 = EMBR_COAP_RESPONSE_CODE(Success, 1),
        Content                 = EMBR_COAP_RESPONSE_CODE(Success, 5),
        // https://datatracker.ietf.org/doc/html/rfc7959#section-6
        Continue                = EMBR_COAP_RESPONSE_CODE(Success, 31),
        NotFound                = EMBR_COAP_RESPONSE_CODE(ClientError, 4),
        NotAcceptable           = EMBR_COAP_RESPONSE_CODE(ClientError, 6),
        // https://datatracker.ietf.org/doc/html/rfc7959#section-6
        RequestEntityIncomplete = EMBR_COAP_RESPONSE_CODE(ClientError, 8),
        UnsupportedContentFormat    = EMBR_COAP_RESPONSE_CODE(ClientError, 15),
        InternalServerError     = EMBR_COAP_RESPONSE_CODE(ServerError, 0),
    };

    enum types : unsigned
    {
        CON,
        NON,
        ACK,
        RST
    };

    static const char* to_string(classes);
    static const char* to_string(codes);
};


constexpr header_base::classes get_class(header_base::codes c)
{
    return static_cast<header_base::classes>(c >> 5);   // NOLINT
}

inline const char* to_string(header_base::classes c) { return header_base::to_string(c); }
inline const char* to_string(header_base::codes c) { return header_base::to_string(c); }

const char* to_string(header_base::types);

}   // namespace embr::coap::internal
