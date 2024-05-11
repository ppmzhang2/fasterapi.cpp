#include "httpreq_message.hpp"
#include "common.hpp"
#include <iostream>
#include <stdexcept>

// TODO: remove initialization of `conn` can significantly increase the
//       IO capacity of the server. It is not safe and we don't know why.
HttpReq::Message::Message(const std::string &req_str)
    : kv({
          std::pair<std::string, std::string>{kK_PATH, ""},
          std::pair<std::string, std::string>{kK_BODY, ""},
      }),
      length(0), method(Method::UNKNOWN), protocol(Protocol::UNKNOWN) {

    // Split header and body; raise error if no trailing two CRLFs
    size_t pos_body = req_str.find(CRLF2); // position of start of body
    if (pos_body == std::string::npos) {
        throw std::invalid_argument("Invalid request string");
        exit(1);
    }

    size_t pos_kv;
    // TODO: the current parser `parse_line_req` requires the input ends with
    //       CRLF. Make it more robust.
    std::string reqhdr_str = req_str.substr(0, pos_body + 2);
    TOUPPER_ASCII(reqhdr_str.data());
    // set body if exists
    if (pos_body + 4 < req_str.size()) {
        set_body(req_str.substr(pos_body + 4));
    }

    // Parse the method, path, and protocol
    pos_kv = reqhdr_str.find(CRLF); // position of end of KV pairs
    if (pos_kv != std::string::npos) {
        update_reqline(reqhdr_str.substr(0, pos_kv), pos_kv);
    }

    // Parse the headers
    std::size_t pos_beg = pos_kv + 2;
    update_kv(reqhdr_str, pos_beg);
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
