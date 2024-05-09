#include "httpsrv_listener.hpp"
#include "common.hpp"
#include "httpreq_message.hpp"
#include <iostream>

// Convert `asio::streambuf` to `std::string`
// Note: This function consumes the buffer.
static inline const std::string streambuf2string(asio::streambuf &buffer) {
    std::istream is(&buffer);
    std::string data((std::istreambuf_iterator<char>(is)),
                     std::istreambuf_iterator<char>());
    return data;
}

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
    return "HTTP/1.1 200 OK" CRLF "Date: " + http_date() +
           CRLF "Content-Type: text/plain" CRLF "Content-Length: 12" CRLF +
           (keep_alive ? "Connection: keep-alive" : "Connection: close") +
           CRLF2 "Hello world!";
}

// Constructor: Initialize server parameters and work guard.
HttpSrv::Listener::Listener(const uint16_t port) : port_(port) {}

// Coroutine that continuously listens for incoming TCP connections on a
// specified port. This function sets up an acceptor, listens on port 8080, and
// spawns a new coroutine for each client connection using the handle_client
// coroutine.
asio::awaitable<void> HttpSrv::Listener::Start() {
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
asio::awaitable<void> HttpSrv::Listener::session(asio::ip::tcp::socket socket) {
    // Check if the socket is open before proceeding.
    if (!socket.is_open()) {
        std::cerr << "Socket is not open." << std::endl;
        co_return;
    }

    asio::streambuf req_buf;

    for (;;)
        try {
            // 1. Asynchronously read until the HTTP header delimiter.
            // NOTE: the function does not stop reading once the delimiter is
            // found; it reads until the buffer is full or the delimiter is
            co_await asio::async_read_until(socket, req_buf, CRLF2,
                                            asio::use_awaitable);

            // 2. Parse the request header and perhaps body.
            HttpReq::Message req(streambuf2string(req_buf));

            // 3. Read the remaining body if it exists.
            if (req.unread() > 0) {
                asio::streambuf reqbody_buf;
                asio::error_code ec_read_body;
                co_await asio::async_read(
                    socket, reqbody_buf, asio::transfer_exactly(req.unread()),
                    asio::redirect_error(asio::use_awaitable, ec_read_body));
                if (ec_read_body) {
                    std::cerr
                        << "Error reading body: " << ec_read_body.message()
                        << std::endl;
                    break;
                }
                req.set_body(req.body() + streambuf2string(reqbody_buf));
            }
            req.Print();

            // 3. Create the response message.
            std::string response = prepare_response(req.keep_alive());

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

            // 5. Close the connection if not Keep-Alive by exiting loop
            if (!req.keep_alive()) {
                break;
            }

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
