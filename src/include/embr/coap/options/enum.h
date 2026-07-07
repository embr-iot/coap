#pragma once

#include <estd/cstdint.h>

namespace embr::coap::internal {

// DEBT: Enums here have convention discrepency with header codes
struct option_enum_base
{
    enum extended_modes : uint8_t
    {
        Extended8Bit = 13,
        Extended16Bit = 14,
        Reserved = 15
    };

    enum content_formats : uint16_t
    {
        // As per [1]
        ImageGif                = 21,
        ImageJpeg               = 22,
        ImagePng                = 23,

        // RFC 7252 Section 12.3
        TextPlain               = 0,
        ApplicationLinkFormat   = 40,
        ApplicationXml          = 41,
        ApplicationOctetStream  = 42,
        ApplicationExi          = 47,
        ApplicationJson         = 50,

        // As per [1]
        ApplicationPatchJson    = 51,
        ApplicationMergeJson    = 52,

        // RFC 7049
        ApplicationCbor         = 60,

        // As per [1]
        ApplicationCwt              = 61,
        ApplicationMultipartCore    = 62,
        ApplicationCborSeq          = 63,

        ApplicationCoapGroupJson    = 256,      // RFC 7390
    };


    // https://datatracker.ietf.org/doc/html/rfc7252#section-12.2
    enum numbers
    {
        /// format: opaque
        IfMatch = 1,
        // format: string
        UriHost = 3,
        // format: opaque
        ETag = 4,
        IfNoneMatch = 5,
        // https://tools.ietf.org/html/rfc7641#section-2
        // format: uint 0-3 bytes
        Observe = 6,
        UriPort = 7,
        LocationPath = 8,
        UriPath = 11,
        ContentFormat = 12,
        MaxAge = 14,
        UriQuery = 15,
        Accept = 17,
        LocationQuery = 20,
        // https://tools.ietf.org/html/rfc7959#section-2.1
        // request payload block-wise
        Block2 = 23,
        // response payload block-wise
        Block1 = 27,
        Size2 = 28,         // https://datatracker.ietf.org/doc/html/rfc7959#section-4
        ProxyUri = 35,
        ProxyScheme = 39,
        Size1 = 60,         // https://datatracker.ietf.org/doc/html/rfc7959#section-4

        // https://www.rfc-editor.org/rfc/rfc9175.html
        Echo = 252,
        RequestTag = 292
    };
};

}

namespace embr::coap::options {

using content_formats = internal::option_enum_base::content_formats;
using numbers = internal::option_enum_base::numbers;

}
