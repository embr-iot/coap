#pragma once

#include "../option.h"
#include "stateful.h"

namespace embr::coap::options {

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
errc stateful_decoder::decode_one(Streambuf& in, F&& f)
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
                    // NOTE: Indicates has_payload flag
                    return {};
                }
                else if(c == -1)
                {
                    // DEBT: estd istream has a way to distinguish EOF from not-yet-available,
                    // if I recall correctly.  Remind myself about that.  It's been years
                    // DEBT: Also on EOF it's clunky to return an error code for a very typical
                    // (no payload) scenario
                    return errc::done;
                }

                r code = dld_.decode_byte(c);

                if(code == r::Bad)
                {
                    return errc::bad;
                }
                else if(code == r::Done)
                {
                    state_ = Value;

                    dld_.reset();

                    // this option header is decoded.  Signal we're ready to proceed to next one
                    return errc::cycle;
                }
            }
            break;

        case Value:
        {
            // TODO: Use streambuf::policy::use with fallback to local buffer if need be
            auto data = (const uint8_t*)in.gptr();
            unsigned avail = in.egptr() - data;
            uint16_t& length = dld_.length_;
            uint16_t n = dld_.delta() + current_number_;
            option<> o((numbers)n, length, data);

            o.end = avail >= length;

            in.pubseekoff(length, estd::ios_base::cur);

            // No fancy dispatch here, though you can run through numbers_dispatch yourself
            // with little ceremony.  Be advised also you must pubseekoff (or similar) yourself
            f(o);

            if(o.end)
            {
                current_number_ += dld_.delta();
                state_ = Header;

                dld_.reset();

                // this option header is decoded.  Signal we're ready to proceed to next one
                return errc::cycle;
            }

            length -= avail;
            in.pubsync();

            return errc::again;
        }
    }

    abort();
}

template <ESTD_CPP_CONCEPT(estd::concepts::InStreambuf) Streambuf, class F>
errc stateful_decoder::decode(Streambuf& in, F&& f)
{
    errc err;

    for(;;)
    {
        err = decode_one(in, f);
        if(err != errc::cycle)
        {
            return err;
        }
    }
}

}
