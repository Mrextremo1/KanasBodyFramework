#pragma once

#include <kbf/debug/log_data.hpp>

#include <deque>
#include <string>
#include <string_view>
#include <chrono>
#include <mutex>
#include <format>
#include <sstream>
#include <iomanip>
#include <algorithm>

#undef ERROR

namespace kbf {

    // ------------------------------------------------------------
    // FixedString helper for compile-time string tags
    // ------------------------------------------------------------
    template <size_t N>
    struct FixedString {
        char value[N];

        constexpr FixedString(const char(&str)[N]) {
            std::copy_n(str, N, value);
        }

        constexpr operator std::string_view() const {
            return { value, N - 1 };
        }
    };

    // ------------------------------------------------------------
    // DebugStack
    // ------------------------------------------------------------
    class DebugStack {
    public:
        DebugStack(size_t limit) : limit{ limit } {}

        enum class Color {
            COL_ERROR,
            COL_WARNING,
            COL_INFO,
            COL_DEBUG,
            COL_SUCCESS
        };

        // ------------------------------------------------------------
        // Color helpers
        // ------------------------------------------------------------
        static glm::vec3 getColor(DebugStack::Color col) {
            switch (col) {
            case DebugStack::Color::COL_ERROR:   return { 0.839f, 0.365f, 0.365f };  // #D65D5D
            case DebugStack::Color::COL_WARNING: return { 0.902f, 0.635f, 0.235f };  // #E6A23C
            case DebugStack::Color::COL_INFO:    return { 0.753f, 0.753f, 0.753f };  // #C0C0C0
            case DebugStack::Color::COL_DEBUG:   return { 0.365f, 0.678f, 0.886f };  // #5DADE2
            case DebugStack::Color::COL_SUCCESS: return { 0.451f, 0.776f, 0.424f };  // #73C66C
            }
            return { 1.0f, 1.0f, 1.0f };
        }

        static std::string getColorTypeAsString(DebugStack::Color col) {
            switch (col) {
            case DebugStack::Color::COL_ERROR:   return "ERROR";
            case DebugStack::Color::COL_WARNING: return "WARNING";
            case DebugStack::Color::COL_INFO:    return "INFO";
            case DebugStack::Color::COL_DEBUG:   return "DEBUG";
            case DebugStack::Color::COL_SUCCESS: return "SUCCESS";
            }
            return "UNKNOWN";
        }

        static DebugStack::Color getColorType(glm::vec3 col) {
            if (col == getColor(DebugStack::Color::COL_ERROR))   return DebugStack::Color::COL_ERROR;
            if (col == getColor(DebugStack::Color::COL_WARNING)) return DebugStack::Color::COL_WARNING;
            if (col == getColor(DebugStack::Color::COL_INFO))    return DebugStack::Color::COL_INFO;
            if (col == getColor(DebugStack::Color::COL_DEBUG))   return DebugStack::Color::COL_DEBUG;
            if (col == getColor(DebugStack::Color::COL_SUCCESS)) return DebugStack::Color::COL_SUCCESS;
            return DebugStack::Color::COL_INFO;
        }

        // ------------------------------------------------------------
        // Old push API
        // ------------------------------------------------------------
        void push(LogData logData) {
            std::lock_guard<std::mutex> lock(mux);
            stack.push_back(std::move(logData));

            if (stack.size() > limit && !stack.empty()) {
                stack.pop_front();
            }
        }

        void push(std::string message, DebugStack::Color colour = DebugStack::Color::COL_DEBUG) {
            push(LogData{ message, getColor(colour), DebugStack::now() });
        }

        void pop() {
            std::lock_guard<std::mutex> lock(mux);
            if (!stack.empty()) stack.pop_front();
        }

        const LogData& peek() const {
            std::lock_guard<std::mutex> lock(mux);
            return stack.back();
        }

        void clear() {
            std::lock_guard<std::mutex> lock(mux);
            stack.clear();
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(mux);
            return stack.empty();
        }

        auto begin() { return stack.begin(); }
        auto end() { return stack.end(); }

        auto begin() const { return stack.begin(); }
        auto end()   const { return stack.end(); }

        static inline std::chrono::system_clock::time_point now() noexcept {
            return std::chrono::system_clock::now();
        }

        std::string string() const {
            std::string result;

            for (const auto& log : stack) {
                auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                    log.timestamp.time_since_epoch()
                ).count();

                std::time_t seconds = millis / 1000;
                std::tm local_tm{};

#ifdef _WIN32
                localtime_s(&local_tm, &seconds);
#else
                localtime_r(&seconds, &local_tm);
#endif

                int ms = static_cast<int>(millis % 1000);

                std::ostringstream time;
                time << std::setfill('0')
                    << std::setw(2) << local_tm.tm_hour << ":"
                    << std::setw(2) << local_tm.tm_min << ":"
                    << std::setw(2) << local_tm.tm_sec << ":"
                    << std::setw(4) << ms;

                result += std::format(
                    "[{}] [{}] {}\n",
                    time.str(),
                    getColorTypeAsString(getColorType(log.colour)),
                    log.data
                );
            }

            return result;
        }

        // ------------------------------------------------------------
        // fpush API — new formatted/tagged logging
        // ------------------------------------------------------------

        // Helper: format string
        template <typename... Args>
        static std::string format(std::string_view fmt, Args&&... args) {
            if constexpr (sizeof...(Args) == 0) {
                return std::string(fmt);
            }
            else {
                return std::format(fmt, std::forward<Args>(args)...);
            }
        }

        // -------------------
        // No tag
        // -------------------
        template <typename... Args>
        void fpush(std::format_string<Args...> fmt, Args&&... args) {
            fpush(Color::COL_DEBUG, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void fpush(Color color, std::format_string<Args...> fmt, Args&&... args) {
            push(LogData{
                std::format(fmt, std::forward<Args>(args)...),
                getColor(color),
                now()
                });
        }

        // -------------------
        // With compile-time tag
        // -------------------
        template <FixedString Tag, typename... Args>
        void fpush(std::format_string<Args...> fmt, Args&&... args) {
            fpush<Tag>(Color::COL_DEBUG, fmt, std::forward<Args>(args)...);
        }

        template <FixedString Tag, typename... Args>
        void fpush(Color color, std::format_string<Args...> fmt, Args&&... args) {
            std::string message = std::format(
                "{} {}",
                std::string_view(Tag),
                std::format(fmt, std::forward<Args>(args)...)
            );
            push(LogData{ std::move(message), getColor(color), now() });
        }

    private:
        mutable std::mutex mux;
        size_t limit;
        std::deque<LogData> stack{};
    };

    inline DebugStack DEBUG_STACK{ 10000 };

}