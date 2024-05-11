#pragma once

#include <chrono>
#include <fstream>
#include <string>

namespace Utils {
    static inline const std::string read_file(const std::string &path,
                                              const std::string &root) {
        std::filesystem::path full_path;

        try {
            full_path = std::filesystem::canonical(
                root + (path.back() == '/' ? path + "index.html" : path));
        } catch (const std::exception &e) {
            return "";
        }

        if (!std::filesystem::exists(full_path) ||
            !std::filesystem::is_regular_file(full_path)) {
            return "";
        }

        std::ifstream file(full_path, std::ios::binary | std::ios::ate);
        if (!file)
            return "";

        auto size = file.tellg();
        std::string content(size, '\0');
        file.seekg(0);
        file.read(&content[0], size);
        return content;
    }

    static inline std::string timestamp() {
        auto now = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        std::stringstream ss;
        ss << std::ctime(&now);
        std::string s = ss.str();
        s.pop_back(); // Remove newline
        return s;
    }

} // namespace Utils
