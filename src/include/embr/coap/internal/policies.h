#pragma once

namespace embr::coap::internal {

struct policies_enum
{
    enum policies
    {
        // Presumptive means we presume all input data is available all the time
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
};

}
