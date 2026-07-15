#pragma once

#include "lwip/pbuf.h"

// EXPERIMENTAL
// reimagining of existing embr::lwip::Pbuf
// Shaping up nicely, though it makes one thing could we specialize an existing
// weak_ptr, shared_ptr, unique_ptr?  Probably not, since those constructs aren't
// well-established for estd - and std forbids specialization of these, at least
// according to AI

namespace embr::lwip::inline _pbuf::inline v1 {

template <bool owning>
class pbuf_base
{
protected:
    pbuf* pbuf_;

    pbuf_base(pbuf* p) : pbuf_{p} {}

public:
    pbuf_base(pbuf_base&& move_from) :
        pbuf_{move_from.pbuf_}
    {
        move_from.pbuf_ = nullptr;
    }

    ~pbuf_base()
    {
        if(owning && pbuf_)   pbuf_free(pbuf_);
    }
};

using owning_pbuf = pbuf_base<true>;

// EXPERIMENTAL
class unique_pbuf : public owning_pbuf
{
    using base_type = owning_pbuf;

protected:
    unique_pbuf(pbuf* p) : owning_pbuf{p} {}

public:
    unique_pbuf() = delete;
    unique_pbuf(const unique_pbuf&) = delete;

    static unique_pbuf alloc(pbuf_layer layer, uint16_t length, pbuf_type type)
    {
        return { pbuf_alloc(layer, length, type) };
    }

    static unique_pbuf alloc(uint16_t length, pbuf_type type = PBUF_RAM)
    {
        return { pbuf_alloc(PBUF_TRANSPORT, length, type) };
    }
};

// EXPERIMENTAL
class weak_pbuf : public owning_pbuf
{

};

class shared_pbuf : public owning_pbuf
{
    using base_type = owning_pbuf;

protected:
    shared_pbuf(pbuf* p) : base_type{p} {}

public:
    static shared_pbuf alloc(pbuf_layer layer, uint16_t length, pbuf_type type = PBUF_RAM)
    {
        return { pbuf_alloc(layer, length, type) };
    }

    static shared_pbuf alloc(uint16_t length, pbuf_type type = PBUF_RAM)
    {
        return { pbuf_alloc(PBUF_TRANSPORT, length, type) };
    }

    static shared_pbuf take_ownership(pbuf* p)
    {
        return { p };
    }

    // EXPERIMENTAL
    static shared_pbuf move(pbuf*& p)
    {
        shared_pbuf created{ p };

        p = nullptr;

        return created;
    }

    shared_pbuf() = delete;

    shared_pbuf(const shared_pbuf& copy_from) :
        base_type{copy_from.pbuf_}
    {
        pbuf_ref(pbuf_);
    }

    shared_pbuf(shared_pbuf&& move_from) :
        base_type{std::move(move_from)}
    {
    }

    constexpr bool valid() const { return pbuf_; }

    operator pbuf*() const { return pbuf_; }

    // EXPERIMENTAL
    template <class F>
    void walk(F&& f)
    {
        for(pbuf* i = pbuf_; i != nullptr; i = i->next)
        {
            f(i->payload, i->len);
        }
    }
};

}

inline embr::lwip::_pbuf::v1::shared_pbuf take_ownership(pbuf* p)
{
    return embr::lwip::_pbuf::v1::shared_pbuf::take_ownership(p);
}

template <class ...Args>
inline embr::lwip::_pbuf::v1::unique_pbuf make_unique_pbuf(Args&&...args)
{
    return embr::lwip::_pbuf::v1::unique_pbuf::alloc(std::forward<Args>(args)...);
}
