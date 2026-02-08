#pragma once

#include <bit>
#include <array>
#include <string>
#include <type_traits>

namespace kbf {

    template <typename T>
    std::string to_binary_string(const T& value)
    {
        static_assert(std::is_trivially_copyable_v<T>,
            "T must be trivially copyable");

        auto bytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);

        std::string result;
        result.reserve(sizeof(T) * 8);

        // Most significant byte first
        for (std::size_t i = 0; i < bytes.size(); ++i) {
            unsigned char b = std::to_integer<unsigned char>(bytes[bytes.size() - 1 - i]);
            for (int bit = 7; bit >= 0; --bit) {
                result.push_back((b & (1u << bit)) ? '1' : '0');
            }
        }

        return result;
    }

}
