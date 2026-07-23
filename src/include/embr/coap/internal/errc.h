#pragma once

namespace embr::coap {

// EXPERIMENTAL, I love the idea of using std/estd errc but it may not be a perfect match
enum class errc
{
    done,
    /// in: more data expected for input - might have blocked
    /// out: more data required to output - might have blocked
    /// (similar to resource_unavailable_try_again / EAGAIN)
    again,
    /// state machine is in ready state, requesting additional call
    cycle,
    //more = cycle,
    /// corrupt data OR stream error
    bad,
    /// data was encountered which is recognized, but incorrect for the requested operation
    alternate,
    //bad_data,
    //bad_stream,
    /// unexpected data (or lack of data), perhaps corrupt but recoverable
    warn,
    /// internal problem with encoder/decoder itself
    fail
};

}
