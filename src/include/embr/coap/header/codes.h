#pragma once

namespace embr::coap::internal {

#define EMBR_COAP_RESPONSE_CODE(class, detail)  ( class << 5 | detail )

struct header_base
{
    // TODO: Refactor into just codes

    enum request_codes
    {
        GET = 1,
        POST,
        PUT,
        DELETE,
    };

    enum response_codes
    {
        OK                      = EMBR_COAP_RESPONSE_CODE(2, 00),
        Created                 = EMBR_COAP_RESPONSE_CODE(2, 01),
        Content                 = EMBR_COAP_RESPONSE_CODE(2, 05),
        NotFound                = EMBR_COAP_RESPONSE_CODE(4, 04),
        UnsupportedContentFormat    = EMBR_COAP_RESPONSE_CODE(4, 15),
        InternalServerError     = EMBR_COAP_RESPONSE_CODE(5, 00),
    };

    // TBD
    static const char* to_string(request_codes);
    static const char* to_string(response_codes);
};

}
