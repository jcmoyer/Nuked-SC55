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
    static constexpr Type Max = 0xff;
};

template <>
struct SizeToInt<Size::Word>
{
    using Type = uint16_t;

    static constexpr Type MSB = 0x8000;
    static constexpr Type Max = 0xffff;
};

template <Size Sz>
using SizeToIntType = typename SizeToInt<Sz>::Type;

template <Size Sz>
constexpr SizeToIntType<Sz> MSB = SizeToInt<Sz>::MSB;

// Widens an integer type by returning the next largest integer type. This type
// only allows widening from uint8_t to uint16_t and from uint16_t to uint32_t
// since these are the only integers widths used in the VM.
template <typename IntType>
struct Widen;

template <>
struct Widen<uint8_t>
{
    using Type = uint16_t;
};

template <>
struct Widen<uint16_t>
{
    using Type = uint32_t;
};

template <typename IntType>
using WidenType = typename Widen<IntType>::Type;

constexpr uint8_t RotateRight(uint8_t x, uint8_t new_msb)
{
    return static_cast<uint8_t>((x >> 1) | (new_msb << 7));
}

constexpr uint16_t RotateRight(uint16_t x, uint16_t new_msb)
{
    return static_cast<uint16_t>((x >> 1) | (new_msb << 15));
}

constexpr uint8_t RotateLeft(uint8_t x, uint8_t new_lsb)
{
    return static_cast<uint8_t>((x << 1) | new_lsb);
}

constexpr uint16_t RotateLeft(uint16_t x, uint16_t new_lsb)
{
    return static_cast<uint16_t>((x << 1) | new_lsb);
}

} // namespace decoder2
