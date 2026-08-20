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

    constexpr bool at_end() const { return current_ && traits::is_null(*current_); }

    constexpr const bc* current() const { return current_; }

    void reset() { current_ = nullptr; }

    /// Investigate 'current' to see if its children match 'v'
    /// @param v
    /// @param top top-level node nav tree
    /// @param current nullptr (for virtual root node) otherwise node whose children to search
    /// @return
    /// @remarks Remember, breadcrumbs specifically do not search grandchildren too.  It's one generation at a time.
    // Keep this in the helper to avoid ADL things
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
        // FIX: Non-match on a child here yields a nullptr which then resets us back to root.  Will
        // create false positives.  Can't compare top_ == current_ either since that's a valid possibility
        // (first child matched from virtual root).  Maybe we can cheat and set current_ to (top_ - 1)?
        const bc* child = search_children(v, top_, current_);

        //if(child != nullptr)
        current_ = child;

        return child;
    }
};

}
