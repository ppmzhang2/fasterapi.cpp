#pragma once

#include <array>
#include <string>

namespace HttpHdr {

    enum class Version : uint8_t {
        UNKNOWN = 0,
        HTTP_1_0,
        HTTP_1_1,
        HTTP_2_0,
    };

    enum class Status : uint8_t {
        InternalServerError = 0,
        NotFound,
        BadRequest,
        OK,
    };

    enum class ContType : uint8_t {
        UNKNOWN = 0,
        TEXT_PLAIN,
        TEXT_HTML,
        TEXT_CSS,
        TEXT_JAVASCRIPT,
        IMAGE_JPEG,
        IMAGE_PNG,
        IMAGE_GIF,
        IMAGE_SVG,
        IMAGE_ICON,
        APPLICATION_JSON,
        APPLICATION_XML,
        APPLICATION_ZIP,
        APPLICATION_PDF,
    };

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

    enum class Conn : uint8_t {
        CLOSE = 0,
        KEEP_ALIVE,
    };

    namespace {
        inline static constexpr char kDEFAULT[] = "UNKNOWN";

        inline static constexpr uint8_t kNVersion = 4;
        inline static constexpr std::array<const char *, kNVersion>
            kArrVersionStr = {kDEFAULT, "HTTP/1.0", "HTTP/1.1", "HTTP/2"};

        inline static constexpr uint8_t kNStatus = 4;
        inline static constexpr std::array<uint16_t, kNStatus> kArrStatusCode =
            {500, 404, 400, 200};
        // can have lower case text as this is for response only
        inline static constexpr std::array<const char *, kNStatus>
            kArrStatusStr = {"500 Internal Server Error", "404 Not Found",
                             "400 Bad Request", "200 OK"};

        inline static constexpr uint8_t kNMethod = 8;
        inline static constexpr std::array<const char *, kNMethod>
            kArrMethodStr = {kDEFAULT, "GET",  "POST",    "PUT",
                             "DELETE", "HEAD", "OPTIONS", "PATCH"};

        inline static constexpr uint8_t kNConn = 3;
        inline static constexpr std::array<const char *, kNConn> kArrConnStr = {
            kDEFAULT, "KEEP-ALIVE", "CLOSE"};

        inline static constexpr uint8_t kNContentType = 14;
        inline static constexpr std::array<const char *, kNContentType>
            kArrContentTypeStr = {
                kDEFAULT,          "TEXT/PLAIN",       "TEXT/HTML",
                "TEXT/CSS",        "TEXT/JAVASCRIPT",  "IMAGE/JPEG",
                "IMAGE/PNG",       "IMAGE/GIF",        "IMAGE/SVG+XML",
                "IMAGE/X-ICON",    "APPLICATION/JSON", "APPLICATION/XML",
                "APPLICATION/ZIP", "APPLICATION/PDF"};

    } // namespace

    static inline const std::string ver2str(Version version) {
        return kArrVersionStr.at(static_cast<uint8_t>(version));
    }
    static inline Version str2ver(const std::string &protocol) {
        for (uint8_t i = 0; i < kNVersion; ++i) {
            if (kArrVersionStr[i] == protocol) {
                return static_cast<Version>(i);
            }
        }
        return Version::UNKNOWN;
    }

    static inline const std::string status2str(Status status) {
        return kArrStatusStr.at(static_cast<uint8_t>(status));
    }

    static inline const std::string method2str(const Method &method) {
        return kArrMethodStr.at(static_cast<uint8_t>(method));
    }
    static inline Method str2method(const std::string &method) {
        for (uint8_t i = 0; i < kNMethod; ++i) {
            if (kArrMethodStr[i] == method) {
                return static_cast<Method>(i);
            }
        }
        return Method::UNKNOWN;
    }

    static inline const std::string conn2str(const Conn &conn) {
        return kArrConnStr.at(static_cast<uint8_t>(conn));
    }
    static inline Conn str2conn(const std::string &conn) {
        for (uint8_t i = 0; i < kNConn; ++i) {
            if (kArrConnStr[i] == conn) {
                return static_cast<Conn>(i);
            }
        }
        return Conn::CLOSE;
    }

    static inline const std::string conttype2str(ContType cont_type) {
        return kArrContentTypeStr.at(static_cast<uint8_t>(cont_type));
    }

} // namespace HttpHdr
