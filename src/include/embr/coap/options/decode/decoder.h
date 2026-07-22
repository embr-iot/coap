#include "fwd.h"
#include "traits.h"

//#include "../../internal/accumulator.h"
#include "../../internal/errc.h"
#include "../../internal/policies.h"

#include <estd/cstdint.h>
#include <estd/streambuf.h>

namespace embr::coap::options {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
class decoder : public internal::policies_enum
{
public:
    using traits = Traits;
    using streambuf_type = estd::remove_cvref_t<Streambuf>;
    static constexpr policies policy = traits::policy;

private:
    Streambuf in_;


    // TODO: policy is still too experimental and doesn't compile in this circumstance
    // See https://github.com/malachi-iot/estdlib/issues/219
    //using use = typename streambuf_type::impl_type::policy::use;

    // DEBT: Do Retry
    template <class F>
    errc emit(F&&, numbers, unsigned len);

    template <numbers n, class F>
    errc emit(F&&, unsigned len, const uint8_t* value);

    template <numbers, class F, class Retry>
    errc dispatch_ll(F&&, unsigned len, Retry&&);

    template <class F, class NoMatchFunctor, class Retry = estd::monostate>
    errc dispatch_ll(F&&, NoMatchFunctor&&, numbers number, unsigned len, Retry&& = {});

public:
    template <class ...Args>
    constexpr decoder(Args&&...args) : in_{std::forward<Args>(args)...} {}

    template <class F, class NoMatchFunctor = estd::monostate>
    errc dispatch(F&&, bool* has_payload, NoMatchFunctor&& = {});

    template <class F>
    errc decode(F&&, bool* has_payload);

    // Route matched and unmatched options through the same functor
    template <class F>
    errc decode_combined(F&& f, bool* has_payload)
    {
        return dispatch(std::forward<F>(f), has_payload, std::forward<F>(f));
    }
};

}
