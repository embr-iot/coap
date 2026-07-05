#pragma once

namespace embr::coap::internal {

#define EMBR_COAP_RESPONSE_CODE(class, detail)  ( class << 5 | detail )

struct header_base
{
    enum request_codes
    {
        GET = 1,
        POST,
        PUT,
        DELETE
    };

    enum response_codes
    {
        CREATED                 = EMBR_COAP_RESPONSE_CODE(2, 01),
        CONTENT                 = EMBR_COAP_RESPONSE_CODE(2, 05),
        NOT_FOUND               = EMBR_COAP_RESPONSE_CODE(4, 04),
        UNSUPPORTED_CONTENT_FORMAT  = EMBR_COAP_RESPONSE_CODE(4, 15),
        INTERNAL_SERVER_ERROR   = EMBR_COAP_RESPONSE_CODE(5, 00),
    };
};

}
