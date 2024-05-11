#pragma once

#include "httphdr.hpp"
#include "httpreq_message.hpp"
#include "utils.hpp"
#include <asio.hpp>

namespace HttpRsp {

    struct Message {
        std::string body;
        HttpHdr::Conn conn;
        HttpHdr::Status code;
        HttpHdr::ContType cont_type;
        char pad[5];

        Message()
            : conn(HttpHdr::Conn::CLOSE),
              code(HttpHdr::Status::InternalServerError),
              cont_type(HttpHdr::ContType::TEXT_PLAIN) {}

        // Serialize the response message to a string.
        const std::string ToStr() const {
            HttpHdr::Version ver = HttpHdr::Version::HTTP_1_1;
            return ver2str(ver) + " " + status2str(code) +
                   CRLF "Date: " + Utils::timestamp() +
                   CRLF "Content-Type: " + conttype2str(cont_type) +
                   CRLF "Content-Length: " + std::to_string(body.size()) +
                   CRLF "Connection: " + conn2str(conn) + CRLF2 + body;
        }

        // Serve the file at the given path.
        // Favor updating the existing response message over creating a new
        // one.
        inline void ServFile(const HttpReq::Message &req,
                             const std::string &root) {

            conn = req.conn == HttpHdr::Conn::KEEP_ALIVE
                       ? HttpHdr::Conn::KEEP_ALIVE
                       : HttpHdr::Conn::CLOSE;

            if (req.method != HttpHdr::Method::GET) {
                code = HttpHdr::Status::BadRequest;
                body = "You are in the wrong place!";
            }

            const std::string cont = Utils::read_file(req.path(), root);
            if (cont.empty()) {
                code = HttpHdr::Status::NotFound;
                body = "404 Not Found";
                cont_type = HttpHdr::ContType::TEXT_PLAIN;
            } else {
                code = HttpHdr::Status::OK;
                body = cont;
                cont_type = HttpHdr::ContType::TEXT_HTML;
            }
        }
    };

} // namespace HttpRsp
