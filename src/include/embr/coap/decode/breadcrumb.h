#pragma once

#include <cstring>  // DEBT: breadcrumb needs this
#include <embr/internal/breadcrumb.h>

// DEBT: This will go up into embr proper

namespace embr {

template <class Breadcrumb = embr::internal::breadcrumb>
class breadcrumb_matcher
{
    using bc = Breadcrumb;

    const bc* top_;
    const bc* current_;

    using traits = embr::internal::breadcrumb_traits<bc>;

public:
    constexpr breadcrumb_matcher(const bc* top) : top_{top}, current_{} {}

    //constexpr bool at_end() const { return current_ && traits::is_null(*current_); }

    constexpr const bc* current() const { return current_; }

    void reset() { current_ = nullptr; }

    /// Investigate to see current node's children match 'v'
    template <class String>
    ESTD_CPP_CONSTEXPR(14) const bc* search(const String& v)
    {
        const bc* child = search_children(v, top_, current_);

        // If we have a valid child, then retain that as the new current parent for next search.
        // Otherwise, keep routing through same current_ over and over, thus always yielding nullptr
        // indicating end of search.  This way it's not UB if someone calls search again and again
        // even if match path is not available.
        if(child != nullptr)    current_ = child;

        return child;
    }

    ESTD_CPP_CONSTEXPR(17) bool invariant() const
    {
        if(!top_)   return false;

        // DEBT: Put an invariant() call into breadcrumb and traits itself, if practical
        return current_ == nullptr || current_ >= top_;
    }

    estd::optional<int> id() const
    {
        return current_ ? current_->id() : estd::nullopt;
    }
};

}
