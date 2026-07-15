#pragma once

#include "lwip/pbuf.h"

// EXPERIMENTAL
// reimagining of existing embr::lwip::Pbuf

namespace embr::lwip::inline _pbuf::inline v1 {

class shared_pbuf
{
    pbuf* pbuf_;

protected:
    shared_pbuf(pbuf* p) : pbuf_{p} {}

public:
    static shared_pbuf take_ownership(pbuf* p)
    {
        return { p };
    }

    shared_pbuf() = delete;

    shared_pbuf(shared_pbuf&& move_from) :
        pbuf_{move_from.pbuf_}
    {
        move_from.pbuf_ = nullptr;
    }

    ~shared_pbuf()
    {
        if(pbuf_)   pbuf_free(pbuf_);
    }

    operator pbuf*() const { return pbuf_; }
};

}

inline embr::lwip::_pbuf::v1::shared_pbuf take_ownership(pbuf* p)
{
    return embr::lwip::_pbuf::v1::shared_pbuf::take_ownership(p);
}
