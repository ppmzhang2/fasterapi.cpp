#include "httprsp_listener.hpp"
#include "common.hpp"
#include "httpreq_message.hpp"
#include "httprsp_message.hpp"
#include <iostream>

// Convert `asio::streambuf` to `std::string`
// Note: This function consumes the buffer.
static inline const std::string streambuf2string(asio::streambuf &buffer) {
    std::istream is(&buffer);
    std::string data((std::istreambuf_iterator<char>(is)),
                     std::istreambuf_iterator<char>());
    return data;
}

// Coroutine that continuously listens for incoming TCP connections on a
// specified port. This function sets up an acceptor, listens on port 8080, and
// spawns a new coroutine for each client connection using the handle_client
// coroutine.
asio::awaitable<void> HttpRsp::Listener::Start() {
    // Get the executor associated with the current coroutine.
    auto exor = co_await asio::this_coro::executor;

    // Set up the TCP acceptor to listen on port 8080 using IPv4.
    asio::ip::tcp::acceptor acceptor(exor, {asio::ip::tcp::v4(), port_});
    std::cout << "Server listening on port " << port_ << std::endl;

    for (;;)
        try {
            // Asynchronously accept a new connection.
            asio::ip::tcp::socket socket =
                co_await acceptor.async_accept(asio::use_awaitable);

            // Spawn a new coroutine to handle the client using a strand for
            // safe concurrent access.
            asio::co_spawn(exor, session(std::move(socket)), asio::detached);
        }
        // Handle exceptions thrown by the acceptor.
        // TODO: [asio.system:24] Too many open files
        catch (std::system_error &e) {
            std::cerr << "Acceptor Exception: "
                      << "[" << e.code() << "] meaning [" << e.what() << "]"
                      << std::endl;
        }
}

// Coroutine to handle an individual client connection, now with Keep-Alive
// support.
asio::awaitable<void> HttpRsp::Listener::session(asio::ip::tcp::socket socket) {
    asio::streambuf req_buf;
    HttpReq::Message req;
    HttpRsp::Message rsp;

    for (;;)
        try {
            // 1. Asynchronously read until the HTTP header delimiter.
            // NOTE: the function does not stop reading once the delimiter is
            // found; it reads until the buffer is full or the delimiter is
            co_await asio::async_read_until(socket, req_buf, CRLF2,
                                            asio::use_awaitable);

            // 2. Parse the request header and perhaps body.
            req.Update(streambuf2string(req_buf));

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

            // 4. Update response message.
            rsp.ServFile(req, root_);

            // 4. Asynchronously write the response back to the client.
            asio::error_code ec_write;
            co_await asio::async_write(
                socket, asio::buffer(rsp.ToStr()),
                asio::redirect_error(asio::use_awaitable, ec_write));
            if (ec_write) {
                std::cerr << "Client closed connection: [" << ec_write.message()
                          << "]" << std::endl;
                break;
            }

            // 5. Close the connection if not Keep-Alive by exiting loop
            if (rsp.conn != HttpHdr::Conn::KEEP_ALIVE) {
                break;
            }

        } catch (std::system_error &e) {
            // Ignore EOF and connection reset errors.
            if (e.code() != asio::error::eof &&
                e.code() != asio::error::connection_reset) {
                std::cerr << "Client Handling Exception: "
                          << "[" << e.code()
                          << "] meaning "
                             "["
                          << e.what() << "]" << std::endl;
            }
        }

    // Attempt graceful closure of the connection
    // TODO:
    // - [asio.system:107]: Transport endpoint is not connected
    try {
        socket.shutdown(asio::ip::tcp::socket::shutdown_send);
        socket.close();
    } catch (std::system_error &e) {
        std::cerr << "Closing Exception: "
                  << "[" << e.code() << "] meaning [" << e.what() << "]"
                  << std::endl;
    }
}
