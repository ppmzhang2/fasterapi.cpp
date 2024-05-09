#pragma once

#include <asio.hpp>
#include <stdint.h>

namespace HttpSrv {

    // Start the server (begin listening for incoming connections).
    void Run(uint16_t, uint16_t);

} // namespace HttpSrv
