#include "server.hpp"
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

using namespace FasterAPI;

// Constructor: Initialize server parameters and work guard.
Server::Server(uint16_t port, uint16_t n_thread)
    : io_ctx_(), work_guard_(asio::make_work_guard(io_ctx_)), port_(port),
      n_thread_(n_thread) {}

std::string Server::http_date() {
    auto now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::ctime(&now);
    std::string s = ss.str();
    s.pop_back(); // Remove newline
    return s;
}

std::string Server::prepare_response(bool keep_alive) {
    return "HTTP/1.1 200 OK\r\n"
           "Date: " +
           http_date() +
           "\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: 12\r\n" +
           (keep_alive ? "Connection: keep-alive\r\n"
                       : "Connection: close\r\n") +
           "\r\n"
           "Hello world!";
}

// Parse the request to check if the client wants to keep the connection alive.
// for simplicity, we are ignoring the request details.
bool Server::flag_keep_alive(asio::streambuf &request) {
    std::istream request_stream(&request);
    std::string request_line;
    while (std::getline(request_stream, request_line) &&
           !request_line.empty() && request_line != "\r") {
        if (request_line.find("Connection: keep-alive") != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Coroutine to handle an individual client connection, now with Keep-Alive
// support.
asio::awaitable<void> Server::handle_client(asio::ip::tcp::socket socket) {
    try {
        asio::streambuf request;

        while (true) {
            // 1. Asynchronously read until the HTTP header delimiter.
            std::size_t bytes_transferred = co_await asio::async_read_until(
                socket, request, "\r\n\r\n", asio::use_awaitable);

            // if socket is closed by the client, break the loop
            if (!socket.is_open()) {
                break;
            }

            // 2. Check if the client wants to keep the connection alive.
            bool keep_alive = flag_keep_alive(request);

            // 3. Create the response message.
            std::string response = prepare_response(keep_alive);

            // 4. Asynchronously write the response back to the client.
            asio::error_code ec_write;
            co_await asio::async_write(
                socket, asio::buffer(response),
                asio::redirect_error(asio::use_awaitable, ec_write));
            if (ec_write) {
                if (ec_write == asio::error::connection_reset ||
                    ec_write == asio::error::broken_pipe) {
                    std::cerr
                        << "Client closed connection: " << ec_write.message()
                        << std::endl;
                }
                break;
            }

            // 5. If the connection is not "keep-alive", close the socket.
            if (!keep_alive) {
                break; // Exit the loop to close the socket fully.
            }

            // Clear the buffer for the next request.
            request.consume(bytes_transferred);
        }

    } catch (std::exception &e) {
        // EOF is expected when the client closes the connection.
        // TODO: verify
        if (e.what() != std::string("End of file")) {
            std::cerr << "Client Handling Exception: " << e.what() << std::endl;
        }
    }

    // Attempt graceful closure of the connection
    try {
        socket.shutdown(asio::ip::tcp::socket::shutdown_send);
        socket.close();
    } catch (...) {
        std::cerr << "Error closing socket." << std::endl;
    }
}

// Coroutine that continuously listens for incoming TCP connections on a
// specified port. This function sets up an acceptor, listens on port 8080, and
// spawns a new coroutine for each client connection using the handle_client
// coroutine.
asio::awaitable<void> Server::listener() {
    // Get the executor from the current coroutine's context.
    auto executor = co_await asio::this_coro::executor;

    // Set up the TCP acceptor to listen on port 8080 using IPv4.
    asio::ip::tcp::acceptor acceptor(executor, {asio::ip::tcp::v4(), port_});
    std::cout << "Server listening on port " << port_ << std::endl;

    while (true) {
        // Asynchronously accept a new connection.
        asio::ip::tcp::socket socket =
            co_await acceptor.async_accept(asio::use_awaitable);

        // Spawn a new coroutine to handle the client using a strand for safe
        // concurrent access.
        asio::co_spawn(asio::make_strand(io_ctx_),
                       handle_client(std::move(socket)), asio::detached);
    }
}

void Server::Run() {
    try {
        // Launch the listener coroutine. This function is responsible for
        // accepting incoming connections and spawning a new coroutine for
        // each connection using asio::co_spawn.
        asio::co_spawn(io_ctx_, listener(), asio::detached);

        for (int i = 0; i < n_thread_; i++) {
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
            threads_.emplace_back([this]() { io_ctx_.run(); });
        }

        // Join all threads to the main thread. This ensures that the main
        // function will wait for all I/O operations to complete before
        // exiting, thereby keeping the server alive to handle requests.
        for (auto &t : threads_) {
            t.join();
        }
    } catch (std::exception &e) {
        // Exception handling: Any exceptions thrown within the server will
        // be caught here and logged to stderr.
        std::cerr << "Server Exception: " << e.what() << std::endl;
    }
}
