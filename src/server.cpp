#include "server.hpp"
#include "listener.hpp"
#include <iostream>

void Server::Run(uint16_t port, uint16_t n_thread) {
    Listener listener = Listener(port);

    asio::io_context ctx;

    // Work Guard: Prevents the io_context from running out of work and
    // concluding too early. This is critical in a multi-threaded server
    // to keep the io_context active even if it's temporarily out of
    // work.
    auto work_guard = asio::make_work_guard(ctx);

    // Thread pool to manage multiple worker threads.
    // The number of threads can be determined based on the hardware
    // concurrency level (std::thread::hardware_concurrency) or set
    // statically to avoid excessive threads, as this can lead to
    // diminishing returns and increased resource consumption. Utilizing
    // multi-core processors for concurrent execution of I/O operations.
    std::vector<std::thread> threads;

    try {

        // Launch the listener coroutine. This function is responsible for
        // accepting incoming connections and spawning a new coroutine for
        // each connection using asio::co_spawn.
        asio::co_spawn(ctx, listener.Start(), asio::detached);

        for (int i = 0; i < n_thread; i++) {
            // Each thread runs the io_context. The run() function will
            // block until all work has finished, including asynchronous
            // tasks initiated by active connections and handlers.
            //
            // Note:
            // - The io_context is NOT thread-safe for arbitrary use.
            // Luckily, it is safe to call `run()`, `run_one()`,
            // `poll()`, `poll_one()` member functions concurrently to
            // process asynchronous events.
            // - ensure that other interactions with io_context, such as
            // posting work to it, respect its non-thread-safe
            // characteristics. This is typically achieved by using
            // `strand` objects to serialize access to the io_context.
            threads.emplace_back([&ctx]() { ctx.run(); });
        }

        // Join all threads to the main thread. This ensures that the main
        // function will wait for all I/O operations to complete before
        // exiting, thereby keeping the server alive to handle requests.
        for (auto &t : threads) {
            t.join();
        }
    } catch (std::system_error &e) {
        // Exception handling: Any exceptions thrown within the server will
        // be caught here and logged to stderr.
        std::cerr << "Server Exception: " << e.what() << std::endl;
    }
}
