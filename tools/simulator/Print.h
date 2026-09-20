#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <type_traits>

class Print
{
public:
    virtual ~Print() {}
    virtual size_t write(uint8_t) = 0;

    virtual size_t write(const uint8_t* buffer, size_t size)
    {
        size_t count = 0;
        while (size--)
        {
            count += write(*buffer++);
        }
        return count;
    }

    size_t print(const char* text) { return write(reinterpret_cast<const uint8_t*>(text), strlen(text)); }
    size_t print(char c) { return write(static_cast<uint8_t>(c)); }

    template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, char>>>
    size_t print(T value)
    {
        const std::string text = std::to_string(value);
        return write(reinterpret_cast<const uint8_t*>(text.c_str()), text.size());
    }

    size_t println() { return write(static_cast<uint8_t>('\n')); }
    size_t println(const char* text) { return print(text) + println(); }

    template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, char>>>
    size_t println(T value)
    {
        return print(value) + println();
    }
};
