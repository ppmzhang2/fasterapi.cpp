#include "listener.hpp"
#include <iostream>

std::string http_date() {
    auto now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::ctime(&now);
    std::string s = ss.str();
    s.pop_back(); // Remove newline
    return s;
}

std::string prepare_response(bool keep_alive) {
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
bool flag_keep_alive(asio::streambuf &request) {
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

// Constructor: Initialize server parameters and work guard.
Server::Listener::Listener(const uint16_t port) : port_(port) {}

// Coroutine that continuously listens for incoming TCP connections on a
// specified port. This function sets up an acceptor, listens on port 8080, and
// spawns a new coroutine for each client connection using the handle_client
// coroutine.
asio::awaitable<void> Server::Listener::Start() {
    // Get the executor associated with the current coroutine.
    auto executor = co_await asio::this_coro::executor;

    // Set up the TCP acceptor to listen on port 8080 using IPv4.
    asio::ip::tcp::acceptor acceptor(executor, {asio::ip::tcp::v4(), port_});
    std::cout << "Server listening on port " << port_ << std::endl;

    for (;;)
        try {
            // Asynchronously accept a new connection.
            asio::ip::tcp::socket socket =
                co_await acceptor.async_accept(asio::use_awaitable);

            // Spawn a new coroutine to handle the client using a strand for
            // safe concurrent access.
            asio::co_spawn(executor, session(std::move(socket)),
                           asio::detached);
        }
        // Handle exceptions thrown by the acceptor.
        catch (std::system_error &e) {
            std::cerr << "Acceptor Exception: " << e.what() << std::endl;
        }
}

// Coroutine to handle an individual client connection, now with Keep-Alive
// support.
asio::awaitable<void> Server::Listener::session(asio::ip::tcp::socket socket) {
    // Check if the socket is open before proceeding.
    if (!socket.is_open()) {
        std::cerr << "Socket is not open." << std::endl;
        co_return;
    }

    asio::streambuf request;

    for (;;)
        try {
            // 1. Asynchronously read until the HTTP header delimiter.
            std::size_t bytes_transferred = co_await asio::async_read_until(
                socket, request, "\r\n\r\n", asio::use_awaitable);

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
        } catch (std::system_error &e) {
            // EOF is expected when the client closes the connection.
            // TODO: verify
            if (e.what() != std::string("End of file")) {
                std::cerr << "Client Handling Exception: " << e.what()
                          << std::endl;
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
