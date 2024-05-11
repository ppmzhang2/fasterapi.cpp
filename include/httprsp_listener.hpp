#pragma once

#include <asio.hpp>
#include <stdint.h>

namespace HttpRsp {

    class Listener {
      public:
        // Constructor to initialize the Listener object with the specified
        // IO context and port.
        Listener(const uint16_t port, const std::string &root_dir)
            : port_(port), root_(root_dir) {}

        // Start listening for incoming connections on the specified port.
        asio::awaitable<void> Start();

      private:
        // The port on which the server listens for incoming connections.
        const uint16_t port_;
        const std::string root_;

      private:
        // Handle a single client connection.
        asio::awaitable<void> session(asio::ip::tcp::socket socket);
    };

} // namespace HttpRsp
