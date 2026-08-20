#include "fwd.h"
#include "traits.h"

//#include "../../internal/accumulator.h"
#include "../../internal/errc.h"
#include "../../internal/policies.h"
#include "../option.h"

#include <estd/cstdint.h>
#include <estd/streambuf.h>

namespace embr::coap::options {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class Traits>
class decoder : public internal::policies_enum
{
public:
    using traits = Traits;
    using streambuf_type = estd::remove_cvref_t<Streambuf>;
    using pos_type = typename streambuf_type::pos_type;
    using streambuf_policy = typename streambuf_type::policy;
    static constexpr policies policy = traits::policy;

    using rfc = estd::internal::rfc::rfc2119;

    // DEBT: Crude recreation of what's in estd.  Doing this due to how
    // https://github.com/malachi-iot/estdlib/issues/219 isn't quite stabalized yet
    // and also there's some bugs in acquiring existing policy in the first place
    struct streambuf_policy_manual
    {
        static constexpr bool blocking = false;

        struct use
        {
            static constexpr rfc gptr = rfc::should;
            static constexpr rfc pptr = rfc::must_not;
            static constexpr rfc seekoff = rfc::should;
            static constexpr rfc seekpos = rfc::should;
        };
    };

private:
    Streambuf in_;


    // TODO: policy is still too experimental and doesn't compile in this circumstance
    // See https://github.com/malachi-iot/estdlib/issues/219
    //using use = typename streambuf_type::impl_type::policy::use;

    // DEBT: Do Retry
    template <class F>
    errc emit(F&&, const option<>&);

    template <numbers n, class F>
    errc emit(F&&, unsigned len, const uint8_t* value);

    template <numbers, class F, class Retry>
    errc dispatch_sgetn_ll(F&&, unsigned len, Retry&&);

    template <numbers, class F, class Retry>
    errc dispatch_gptr_ll(F&&, unsigned len, Retry&&);

    template <class F, class NoMatchFunctor, class Retry = estd::monostate>
    errc dispatch_ll(F&&, NoMatchFunctor&&, numbers number, unsigned len, Retry&& = {});

public:
    template <class ...Args>
    explicit constexpr decoder(Args&&...args) : in_{std::forward<Args>(args)...} {}

    template <class F, class NoMatchFunctor = estd::monostate>
    errc dispatch(F&&, bool* has_payload, NoMatchFunctor&& = {});

    /// @param current_number
    /// @return
    static errc decode_one(Streambuf& in, option<>*, uint16_t* current_number);

    /// @param current_number
    /// @return
    errc decode_one(option<>*, uint16_t* current_number);

    template <class F>
    errc decode(F&&);

    // Route matched and unmatched options through the same functor
    template <class F>
    errc dispatch_combined(F&& f, bool* has_payload)
    {
        return dispatch(std::forward<F>(f), has_payload, std::forward<F>(f));
    }
};

}
