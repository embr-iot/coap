#include <estd/streambuf.h>

namespace embr::coap {

template <ESTD_CPP_CONCEPT(estd::concepts::OutStreambuf) Streambuf>
class encoder;

}
