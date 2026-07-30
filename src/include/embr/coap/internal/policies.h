#pragma once

#include <estd/streambuf.h>

namespace embr::coap::internal {

struct policies_enum
{
    // DEBT: I think this is something we can autodeduce
    // See https://github.com/malachi-iot/estdlib/issues/219
    /// How we should treat an in or out Streambuf
    enum policies
    {
        // Presumptive means we presume either:
        // 1. all input data is available by way of gptr all the time
        // 2. complete output data region pptr is available all the time
        Presumptive,
        // Non-contiguous means we operate similarly to Retry.  However, in this mode
        // we still presume all data is immediately available - merely that it takes
        // multiple reads to get it.  This reflects LwIP pbuf/netbuf style where data
        // IS immediately available but requires a boundary hop
        NonContiguous,
        // Retry means if the data we need isn't ready yet, keep retrying until it
        // is (effectively poll-blocking).
        Retry
    };

    // EXPERIMENTAL
    // See https://github.com/malachi-iot/estdlib/issues/219
    template <class Streambuf>
    static constexpr policies deduce_out()
    {
        using rfc = estd::internal::rfc::rfc2119;
        using use = typename Streambuf::policy::use;

        // This isn't really gonna work out.  NonContiguous (such as lwip pbuf)
        // and fully contiguous like spanbuf both are gonna report pptr >= should
        // See https://github.com/malachi-iot/estdlib/issues/219
        return use::pptr >= rfc::should ? Presumptive : NonContiguous;
    }

    // EXPERIMENTAL
    // See https://github.com/malachi-iot/estdlib/issues/219
    template <class Streambuf>
    static constexpr policies deduce_in()
    {
        using rfc = estd::internal::rfc::rfc2119;
        using use = typename Streambuf::policy::use;

        return use::gptr >= rfc::should ? Presumptive : NonContiguous;
    }
};

}
