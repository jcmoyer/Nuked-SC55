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
};

template <>
struct SizeToInt<Size::Word>
{
    using Type = uint16_t;
};

template <Size Sz>
using SizeToIntType = typename SizeToInt<Sz>::Type;

} // namespace decoder2
