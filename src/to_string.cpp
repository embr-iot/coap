#include <embr/coap/decode/decoder.h>

#define CASE(x) case t::x: return #x;

namespace embr::coap::internal {

const char* to_string(decoder::states state)
{
    using t = decoder;

    switch(state)
    {
        CASE(Header)
        CASE(Token)
        CASE(Options)
        CASE(Payload)
        CASE(Done)
    }

    abort();
}

}
