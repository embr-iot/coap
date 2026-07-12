#pragma once

#include "../option.h"
#include "stateful.h"

namespace embr::coap::options {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::errc stateful_decoder::decode_one(Streambuf& in, F&& f)
{
    switch(state_)
    {
        case Header:
            using r = delta_length_decoder::codes;
            for(;;)
            {
                int c = in.sbumpc();
                if(c == 0xFF)
                {
                    // TODO: Indicate has_payload flag
                    return {};
                }
                else if(c == -1)
                {
                    // DEBT: estd istream has a way to distinguish EOF from not-yet-available,
                    // if I recall correctly.  Remind myself about that.  It's been years
                    return errc::resource_unavailable_try_again;
                }

                r code = dlc_.decode_byte(c);

                if(code == r::Bad)
                {
                    return errc::invalid_argument;
                }
                else if(code == r::Done)
                {
                    state_ = Value;

                    return errc::operation_in_progress;
                }
            }
            break;

        case Value:
        {
            // TODO: Use streambuf::policy::use
            auto data = (const uint8_t*)in.gptr();
            unsigned avail = in.egptr() - data;
            uint16_t& length = dlc_.length_;
            uint16_t n = dlc_.delta() + current_number_;
            option<> o((numbers)n, length, data);

            o.end = avail >= length;

            in.pubseekoff(length, estd::ios_base::cur);

            // No fancy dispatch here, though you can run through numbers_dispatch yourself
            // with little ceremony
            f(o);

            if(o.end)
            {
                current_number_ += dlc_.delta();
                state_ = Header;
                return errc::operation_in_progress;
            }

            length -= avail;
            in.pubsync();

            return errc::resource_unavailable_try_again;
        }
    }

    abort();
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
estd::errc stateful_decoder::decode(Streambuf& in, F&& f)
{
    errc err;

    for(;;)
    {
        err = decode_one(in, f);
        if(err != errc::operation_in_progress)
        {
            return err;
        }
    }
}

}
