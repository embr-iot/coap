#pragma once

namespace embr::coap {

// EXPERIMENTAL, I love the idea of using std/estd errc but it may not be a perfect match
enum class errc
{
    done,
    /// in: more data expected for input - would have blocked
    /// out: more data required to output - would have blocked
    /// (similar to resource_unavailable_try_again / EAGAIN)
    again,
    /// state machine is in ready state, requesting additional call
    cycle,
    //more = cycle,
    /// corrupt data OR stream error
    bad,
    //bad_data,
    //bad_stream,
    /// unexpected data, perhaps corrupt but recoverable
    warn
};

}