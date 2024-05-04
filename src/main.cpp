#include "asio.hpp"
#include "asio/experimental/awaitable_operators.hpp"
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/use_awaitable.hpp>
#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define N_THREADS 4

std::string http_date() {
    auto now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::ctime(&now);
    std::string s = ss.str();
    s.pop_back(); // Remove newline
    return s;
}

// Coroutine to handle an individual client connection.
// This function reads a complete HTTP request, logs it, and sends a basic
// response. Parameter:
//    socket - an `asio::ip::tcp::socket` object representing the connection to
//    a client.
asio::awaitable<void> handle_client(asio::ip::tcp::socket socket) {
    try {
        // 1. Buffer to store the incoming data.
        asio::streambuf request;

        // 2. Asynchronously read data from the socket
        // Until the delimiter "\r\n\r\n" is found. This delimiter indicates the
        // end of the HTTP request header.
        co_await asio::async_read_until(socket, request, "\r\n\r\n",
                                        asio::use_awaitable);

        // 3. Log to the console request header

        // 3.1 Stream to process the data in the buffer.
        std::istream request_stream(&request);
        std::string request_line;

        // 3.2 Extract each line from the request stream and log to the console
        // until an empty line is encountered, which indicates the end of the
        // request header.
        while (std::getline(request_stream, request_line) &&
               !request_line.empty()) {
            std::cout << request_line << std::endl;
        }

        // 4. Create a simple HTTP response message with headers and a body.
        std::string response = "HTTP/1.1 200 OK\r\n"
                               "Date: " +
                               http_date() +
                               "\r\n"
                               "Content-Type: text/plain\r\n"
                               "Content-Length: 12\r\n"
                               "\r\n"
                               "Hello world!";

        // 5. Asynchronously send the response back to the client.
        co_await asio::async_write(socket, asio::buffer(response),
                                   asio::use_awaitable);
    } catch (std::exception &e) {
        // If an exception occurs during processing, log it to the standard
        // error stream.
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

// Coroutine that continuously listens for incoming TCP connections on a
// specified port. This function sets up an acceptor, listens on port 8080, and
// spawns a new coroutine for each client connection using the handle_client
// coroutine.
asio::awaitable<void> listener(asio::io_context &io_context) {
    // Get the executor from the current coroutine's context.
    auto executor = co_await asio::this_coro::executor;

    // Set up the TCP acceptor to listen on port 8080 using IPv4.
    asio::ip::tcp::acceptor acceptor(executor, {asio::ip::tcp::v4(), 8080});
    std::cout << "Server listening on port 8080..." << std::endl;

    while (true) {
        // Asynchronously accept a new connection.
        asio::ip::tcp::socket socket =
            co_await acceptor.async_accept(asio::use_awaitable);

        // Spawn a new coroutine to handle the client using a strand for safe
        // concurrent access.
        asio::co_spawn(asio::make_strand(io_context),
                       handle_client(std::move(socket)), asio::detached);
    }
}

int main() {
    try {
        // The io_context is the main object in ASIO that handles all I/O
        // operations.
        asio::io_context io_context;

        // Work Guard: Prevents the io_context from running out of work and
        // concluding too early. This is critical in a multi-threaded server to
        // keep the io_context active even if it's temporarily out of work.
        auto work_guard = asio::make_work_guard(io_context);

        // Launch the listener coroutine. This function is responsible for
        // accepting incoming connections and spawning a new coroutine for each
        // connection using asio::co_spawn.
        asio::co_spawn(io_context, listener(io_context), asio::detached);

        // Create a vector to hold the worker threads that will run the
        // io_context.
        std::vector<std::thread> threads;

        // Initialize threads to run the io_context. The number of threads can
        // be determined based on the hardware concurrency level
        // (std::thread::hardware_concurrency) or set statically to avoid
        // excessive threads, as this can lead to diminishing returns and
        // increased resource consumption.
        // Utilizing multi-core processors for concurrent execution of I/O
        // operations.
        for (int i = 0; i < N_THREADS; i++) {
            threads.emplace_back([&io_context]() {
                // Each thread runs the io_context. The run() function will
                // block until all work has finished, including asynchronous
                // tasks initiated by active connections and handlers.
                //
                // Note:
                // - The io_context is NOT thread-safe for arbitrary use.
                // Luckily, it is safe to call `run()`, `run_one()`, `poll()`,
                // `poll_one()` member functions concurrently to process
                // asynchronous events.
                // - ensure that other interactions with io_context, such as
                // posting work to it, respect its non-thread-safe
                // characteristics. This is typically achieved by using
                // `strand` objects to serialize access to the io_context.
                io_context.run();
            });
        }

        // Join all threads to the main thread. This ensures that the main
        // function will wait for all I/O operations to complete before exiting,
        // thereby keeping the server alive to handle requests.
        for (auto &t : threads) {
            t.join();
        }
    } catch (std::exception &e) {
        // Exception handling: Any exceptions thrown within the server will be
        // caught here and logged to stderr.
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
