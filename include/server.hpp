#pragma once

#include "asio.hpp"
#include <stdint.h>
#include <thread>
#include <vector>

namespace FasterAPI {

    class Server {
      public:
        // Constructor to initialize the server with a given port and number of
        // worker threads.
        Server(uint16_t port, uint16_t n_thread);

        // Start the server (begin listening for incoming connections).
        void Run();

      private:
        // The main object in ASIO that handles all I/O operations.
        asio::io_context io_ctx_;

        // Work Guard: Prevents the io_context from running out of work and
        // concluding too early. This is critical in a multi-threaded server
        // to keep the io_context active even if it's temporarily out of
        // work.
        asio::executor_work_guard<asio::io_context::executor_type> work_guard_;

        // Thread pool to manage multiple worker threads.
        // The number of threads can be determined based on the hardware
        // concurrency level (std::thread::hardware_concurrency) or set
        // statically to avoid excessive threads, as this can lead to
        // diminishing returns and increased resource consumption. Utilizing
        // multi-core processors for concurrent execution of I/O operations.
        std::vector<std::thread> threads_;

        // The port on which the server listens for incoming connections.
        uint16_t port_;

        // The number of worker threads to spawn for processing I/O events
        // concurrently.
        uint16_t n_thread_;

      private:
        // Start listening for incoming TCP connections on the specified port.
        asio::awaitable<void> listener();

        // Coroutine to handle individual client connections.
        asio::awaitable<void> handle_client(asio::ip::tcp::socket socket);

        // Helper functions.
        std::string prepare_response(bool keep_alive);
        bool flag_keep_alive(asio::streambuf &request);
        std::string http_date();
    };

} // namespace FasterAPI
