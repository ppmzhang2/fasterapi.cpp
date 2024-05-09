#include "httpreq_message.hpp"
#include "common.hpp"
#include <iostream>
#include <stdexcept>

HttpReq::Message::Message(const std::string &req_str) {
    size_t pos_body = req_str.find(CRLF2); // position of start of body
    // complain error of invalid argument if no trailing two CRLFs
    if (pos_body == std::string::npos) {
        throw std::invalid_argument("Invalid request string");
        exit(1);
    }

    size_t pos_kv;
    // TODO: the current parser `parse_line_req` requires the input ends with
    //       CRLF. Make it more robust.
    std::string reqhdr_str = req_str.substr(0, pos_body + 2);
    TOUPPER_ASCII(reqhdr_str.data());
    this->set_body(req_str.substr(pos_body + 4));

    // Parse the method, path, and protocol
    pos_kv = reqhdr_str.find(CRLF); // position of end of KV pairs
    if (pos_kv != std::string::npos) {
        this->update_reqline(reqhdr_str.substr(0, pos_kv), pos_kv);
    } else {
        this->method = Method::UNKNOWN;
        this->protocol = Protocol::UNKNOWN;
    }

    // Parse the headers
    std::size_t pos_beg = pos_kv + 2;
    this->update_kv(reqhdr_str, pos_beg);
}

void HttpReq::Message::Print() const {
    std::cout << "Method: " << HttpReq::method2str(method) << std::endl;
    std::cout << "Protocol: " << HttpReq::proto2str(protocol) << std::endl;
    std::cout << "Connection: " << HttpReq::conn2str(conn) << std::endl;
    std::cout << "Content Length: " << length << std::endl;
    std::cout << "Headers: " << std::endl;
    for (const auto &header : kv) {
        std::cout << "  " << header.first << ": " << header.second << std::endl;
    }
}
