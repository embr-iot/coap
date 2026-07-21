#include "fwd.h"

#include "../../internal/policies.h"

#include <estd/cstdint.h>
#include <estd/streambuf.h>

namespace embr::coap::options {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf>
class decoder : public internal::policies_enum
{
    using errc = estd::errc;

    Streambuf in_;

    using streambuf_type = estd::remove_cvref_t<Streambuf>;

    // TODO: policy is still too experimental and doesn't compile in this circumstance
    // See https://github.com/malachi-iot/estdlib/issues/219
    //using use = typename streambuf_type::impl_type::policy::use;

    template <numbers, class F, class Retry>
    estd::errc dispatch_ll(F&&, unsigned len, Retry&&);

    template <class F, class NoMatchFunctor, class Retry = estd::monostate>
    estd::errc dispatch(F&&, NoMatchFunctor&&, numbers number, unsigned len, Retry&& = {});

public:
    template <class ...Args>
    constexpr decoder(Args&&...args) : in_{std::forward<Args>(args)...} {}

    // DEBT: Rename to 'dispatch' to indicate type of decode behavior
    template <class F, class NoMatchFunctor = estd::monostate>
    estd::errc decode(F&&, bool* has_payload, NoMatchFunctor&& = {});

    // Route matched and unmatched options through the same functor
    template <class F>
    estd::errc decode_combined(F&& f, bool* has_payload)
    {
        return decode(std::forward<F>(f), has_payload, std::forward<F>(f));
    }
};

}
