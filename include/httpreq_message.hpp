#pragma once

#include "common.hpp"
#include "httphdr.hpp"
#include <map>
#include <stdint.h>
#include <string>
#include <unordered_map>

namespace HttpReq {

    // Struct to hold HTTP request message
    struct Message {

      private:
        inline static const std::string kK_PATH = "PATH";
        inline static const std::string kK_BODY = "BODY";
        inline static const std::string kK_LENGTH = "CONTENT-LENGTH";
        inline static const std::string kK_CONN = "CONNECTION";

      public:
        std::unordered_map<std::string, std::string> kv;
        size_t length;
        HttpHdr::Method method;
        HttpHdr::Version version;
        HttpHdr::Conn conn;

        // Constructor
        Message()
            : kv({
                  std::pair<std::string, std::string>{kK_PATH, ""},
                  std::pair<std::string, std::string>{kK_BODY, ""},
              }),
              length(0), method(HttpHdr::Method::UNKNOWN),
              version(HttpHdr::Version::UNKNOWN), conn(HttpHdr::Conn::UNKNOWN) {
        }

        // Constructor with a string input
        Message(const std::string &);

        // Get a value by a specified key (read-only)
        const std::string &get(const std::string &key) const {
            auto it = kv.find(key);
            if (it == kv.end()) {
                throw std::out_of_range("Key not found: " + key);
            }
            return it->second;
        }

        // Set a value by a specified key
        void set(const std::string &key, const std::string &value) {
            kv[key] = value;
        }

        // Const accessor for path value (read-only)
        const std::string &path() const { return kv.at(kK_PATH); }

        // Setter (or non-const accessor) to modify the path value
        void set_path(const std::string &new_path) { kv[kK_PATH] = new_path; }

        // Const accessor for "BODY" value (read-only)
        const std::string &body() const { return kv.at("BODY"); }

        // Setter (or non-const accessor) to modify the "BODY" value
        void set_body(const std::string &new_body) { kv["BODY"] = new_body; }

        // Other attributes
        // Get the length of unread data (body)
        // NOTE: based on stress test from wrk, must check if length is zero
        // before calculating unread
        size_t unread() const {
            return length > 0 ? length - body().size() : 0;
        }

        bool keep_alive() const { return conn == HttpHdr::Conn::KEEP_ALIVE; }

        void Print() const;

      private:
        inline void update_reqline(const std::string &, const size_t &);

        inline void update_kv(const std::string &, const size_t &);
    };

    // Function to update the request line.
    // NOTE: will NOT set default values if not found
    inline void Message::update_reqline(const std::string &req_str,
                                        const size_t &pos_end) {
        // Split request line by spaces
        std::size_t pos_dlm_1 = req_str.find(' ');
        std::size_t pos_dlm_2 = req_str.find(' ', pos_dlm_1 + 1);
        if (pos_dlm_1 != std::string::npos && pos_dlm_2 != std::string::npos) {
            method = HttpHdr::str2method(req_str.substr(0, pos_dlm_1));
            version = HttpHdr::str2ver(req_str.substr(pos_dlm_2 + 1, pos_end));
            set_path(req_str.substr(pos_dlm_1 + 1, pos_dlm_2 - pos_dlm_1 - 1));
        }
    }

    // Parse the request string and update the header key-value pairs.
    // NOTE: will NOT set default values if not found
    //
    // Example:
    //
    // ```
    // GET / HTTP/1.1\r\n
    // HOST: EXAMPLE.COM\r\n
    // USER-AGENT: MYAGENT/1.0\r\n
    // CONTENT-LENGTH: 13\r\n
    // ```
    //
    // Each record is separated by EOL (CRLF), and the key-value pair is
    // separated by a colon. The key is case-insensitive.
    //
    // - `pos_beg` will indicate the starting position of the key-value pairs.
    // - two indices will be used in the loop to indicate the current starting
    //   position and the end of a record (`pos_cur` and `pos_eol`).
    inline void Message::update_kv(const std::string &req_str,
                                   const size_t &pos_beg) {
        std::size_t pos_cur = pos_beg;
        std::size_t pos_eol = pos_beg;
        std::string key, val;
        for (;;) {
            pos_eol = req_str.find(CRLF, pos_cur);
            // break updating loop if no more EOL is found
            if (pos_eol == std::string::npos) {
                break;
            }
            // Find the separator between header name and value
            std::size_t pos_dlm = req_str.find(':', pos_cur);
            if (pos_dlm != std::string::npos) {
                key = req_str.substr(pos_cur, pos_dlm - pos_cur);
                val = req_str.substr(pos_dlm + 2, pos_eol - pos_dlm - 2);
                set(key, val);
            }
            // start from the next line
            pos_cur = pos_eol + 2;
        }

        // fill length and connection
        try {
            const std::string len_str = get(kK_LENGTH);
            length = std::stoi(len_str);
            kv.erase(kK_LENGTH);
        } catch (const std::out_of_range &e) {
            // ignore
        }
        try {
            const std::string conn_str = get(kK_CONN);
            conn = HttpHdr::str2conn(conn_str);
            kv.erase(kK_CONN);
        } catch (const std::out_of_range &e) {
            // ignore
        }
    }

} // namespace HttpReq
