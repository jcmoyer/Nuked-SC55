#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

// Allows using string_view with unordered_map<string, ...>
struct TransparentStringHash
{
    using is_transparent = void;

    size_t operator()(const std::string& s) const
    {
        return std::hash<std::string>{}(s);
    }

    size_t operator()(std::string_view s) const
    {
        return std::hash<std::string_view>{}(s);
    }
};

template <typename ValueType>
using StringMap = std::unordered_map<std::string, ValueType, TransparentStringHash, std::equal_to<void>>;
