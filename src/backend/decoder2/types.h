#pragma once

#include <cstdint>

namespace decoder2
{

enum class Size : uint8_t
{
    Byte,
    Word,
};

template <Size Sz>
struct SizeToInt;

template <>
struct SizeToInt<Size::Byte>
{
    using Type = uint8_t;

    static constexpr Type MSB = 0x80;
};

template <>
struct SizeToInt<Size::Word>
{
    using Type = uint16_t;

    static constexpr Type MSB = 0x8000;
};

template <Size Sz>
using SizeToIntType = typename SizeToInt<Sz>::Type;

template <Size Sz>
constexpr SizeToIntType<Sz> MSB = SizeToInt<Sz>::MSB;

} // namespace decoder2
