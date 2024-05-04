#include "asio.hpp"
#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

std::string http_date() {
    auto now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::ctime(&now);
    std::string s = ss.str();
    s.pop_back(); // Remove newline
    return s;
}

void handle_client(asio::ip::tcp::socket &socket) {
    try {
        asio::streambuf request;
        asio::read_until(socket, request, "\r\n\r\n");

        // Log request
        std::istream request_stream(&request);
        std::string request_line;
        while (std::getline(request_stream, request_line) &&
               request_line != "\r") {
            std::cout << request_line << "\n";
        }

        // Send response
        std::string response = "HTTP/1.1 200 OK\r\n"
                               "Date: " +
                               http_date() +
                               "\r\n"
                               "Content-Type: text/plain\r\n"
                               "Content-Length: 12\r\n"
                               "\r\n"
                               "Hello world!";

        asio::write(socket, asio::buffer(response));
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

int main() {
    try {
        asio::io_context io_context;

        asio::ip::tcp::acceptor acceptor(
            io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8080));
        std::cout << "Server listening on port 8080...\n";

        while (true) {
            asio::ip::tcp::socket socket(io_context);
            acceptor.accept(socket);
            handle_client(socket);
        }
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
