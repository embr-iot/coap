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

    /// Investigate 'current' to see if its children match 'v'
    /// @param v
    /// @param top top-level node nav tree
    /// @param current nullptr (for virtual root node) otherwise node whose children to search
    /// @return
    /// @remarks Remember, breadcrumbs specifically do not search grandchildren too.  It's one generation at a time.
    // Keep this in the helper to avoid ADL things.  Consider putting this up at embr proper
    template <class String>
    ESTD_CPP_CONSTEXPR(17) static const bc* search_children(const String& v, const bc* top, const bc* current)
    {
        // When just starting, pretend to have root node so search siblings without a child
        current = current == nullptr ? top : first_child(current);
        return internal::search_siblings(current, v);
    }

    /// Investigate to see current node's children match 'v'
    template <class String>
    ESTD_CPP_CONSTEXPR(17) const bc* search(const String& v)
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
};

}
