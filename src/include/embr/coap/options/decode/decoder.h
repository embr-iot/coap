#include "fwd.h"

#include <estd/cstdint.h>
#include <estd/istream.h>

namespace embr::coap::options {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
class decoder
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

    using errc = estd::errc;

    Streambuf in_;

    template <numbers, class F, class Retry>
    estd::errc dispatch_ll(F&&, unsigned len, Retry&&);

    template <class F, class Retry = estd::monostate>
    estd::errc dispatch(F&&, numbers number, unsigned len, Retry&& = {});

public:
    template <class ...Args>
    constexpr decoder(Args&&...args) : in_{std::forward<Args>(args)...} {}

    template <class F>
    estd::errc decode(F&&);
};

}
