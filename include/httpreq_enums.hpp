#pragma once

#include <array>
#include <stdint.h>
#include <string>

namespace HttpReq {

    // ------------------------------------------------------------------------
    // Enumerated classes for HTTP request
    //
    // - method, protocol, connection, etc.
    // ------------------------------------------------------------------------

    enum class Method : uint8_t {
        UNKNOWN = 0,
        GET,
        POST,
        PUT,
        DELETE,
        HEAD,
        OPTIONS,
        PATCH,
    };

    enum class Protocol : uint8_t {
        UNKNOWN = 0,
        HTTP_1_0,
        HTTP_1_1,
        HTTP_2,
    };

    enum class Conn : uint8_t {
        UNKNOWN = 0,
        KEEP_ALIVE,
        CLOSE,
    };

    namespace {
        inline static constexpr std::string_view kDEFAULT = "UNKNOWN";

        inline static constexpr uint8_t kMethodSize = 8;
        inline static constexpr std::array<std::string_view, kMethodSize>
            kArrMethodStr = {kDEFAULT, "GET",  "POST",    "PUT",
                             "DELETE", "HEAD", "OPTIONS", "PATCH"};

        inline static constexpr uint8_t kProtocolSize = 4;
        inline static constexpr std::array<std::string_view, kProtocolSize>
            kArrProtocolStr = {kDEFAULT, "HTTP/1.0", "HTTP/1.1", "HTTP/2"};

        inline static constexpr uint8_t kConnSize = 3;
        inline static constexpr std::array<std::string_view, kConnSize>
            kArrConnStr = {kDEFAULT, "KEEP-ALIVE", "CLOSE"};

    } // namespace

    // ------------------------------------------------------------------------
    // Helper functions to convert between string and enumerated classes
    // ------------------------------------------------------------------------

    // Function to convert string to Method using kArrMethodStr
    static inline Method str2method(const std::string_view &method) {
        for (uint8_t i = 0; i < kMethodSize; ++i) {
            if (kArrMethodStr[i] == method) {
                return static_cast<Method>(i);
            }
        }
        return Method::UNKNOWN;
    }

    // Function to convert Method to string using kArrMethodStr
    static inline const std::string_view method2str(const Method &method) {
        return kArrMethodStr.at(static_cast<uint8_t>(method));
    }

    static inline Protocol str2proto(const std::string_view &protocol) {
        for (uint8_t i = 0; i < kProtocolSize; ++i) {
            if (kArrProtocolStr[i] == protocol) {
                return static_cast<Protocol>(i);
            }
        }
        return Protocol::UNKNOWN;
    }

    static inline const std::string_view proto2str(const Protocol &protocol) {
        return kArrProtocolStr.at(static_cast<uint8_t>(protocol));
    }

    static inline Conn str2conn(const std::string_view &conn) {
        for (uint8_t i = 0; i < kConnSize; ++i) {
            if (kArrConnStr[i] == conn) {
                return static_cast<Conn>(i);
            }
        }
        return Conn::UNKNOWN;
    }

    static inline const std::string_view conn2str(const Conn &conn) {
        return kArrConnStr.at(static_cast<uint8_t>(conn));
    }

} // namespace HttpReq
