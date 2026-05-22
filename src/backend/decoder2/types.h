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

    using SignedType = int8_t;

    static constexpr SignedType SignedMin = INT8_MIN;
    static constexpr SignedType SignedMax = INT8_MAX;
};

template <>
struct SizeToInt<Size::Word>
{
    using Type = uint16_t;

    static constexpr Type MSB = 0x8000;
    static constexpr Type Max = 0xffff;

    using SignedType = int16_t;

    static constexpr SignedType SignedMin = INT16_MIN;
    static constexpr SignedType SignedMax = INT16_MAX;
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

template <typename IntType>
struct MakeSigned;

template <>
struct MakeSigned<uint8_t>
{
    using Type = int8_t;
};

template <>
struct MakeSigned<uint16_t>
{
    using Type = int16_t;
};

template <>
struct MakeSigned<uint32_t>
{
    using Type = int32_t;
};

template <typename IntType>
using MakeSignedType = typename MakeSigned<IntType>::Type;

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

template <Size Sz>
struct BinopResult
{
    // result of operation *unsigned*
    SizeToIntType<Sz> result_bits;

    // N, Z, V, C flags respectively
    bool negative;
    bool zero;
    bool overflow;
    bool carry;
};

enum class Binop
{
    Add,
    Subtract,
};

// Computes the result of `a Op b` and information about the operation.
template <Size Sz, Binop Op>
constexpr BinopResult<Sz> GenericBinop(SizeToIntType<Sz> a, SizeToIntType<Sz> b, bool carry = false)
{
    using OpUnsigned   = SizeToIntType<Sz>;
    using OpSigned     = MakeSignedType<OpUnsigned>;
    using WideUnsigned = WidenType<OpUnsigned>;
    using WideSigned   = MakeSignedType<WideUnsigned>;

    WideUnsigned result_u;
    WideSigned   result_s;

    if constexpr (Op == Binop::Add)
    {
        result_u = (WideUnsigned)(a + b) + (WideUnsigned)carry;
        result_s = (WideSigned)((OpSigned)a + (OpSigned)b) + (WideSigned)carry;
    }
    else if constexpr (Op == Binop::Subtract)
    {
        result_u = (WideUnsigned)(a - b) - (WideUnsigned)carry;
        result_s = (WideSigned)((OpSigned)a - (OpSigned)b) - (WideSigned)carry;
    }

    return {
        .result_bits = (OpUnsigned)result_u,
        .negative    = (result_u & MSB<Sz>) != 0,
        .zero        = (OpUnsigned)result_u == 0,
        .overflow    = result_s < SizeToInt<Sz>::SignedMin || result_s > SizeToInt<Sz>::SignedMax,
        .carry       = (result_u & (MSB<Sz> << 1)) != 0,
    };
}

// Returns the result of `a + b`.
template <Size Sz>
constexpr BinopResult<Sz> GenericAdd(SizeToIntType<Sz> a, SizeToIntType<Sz> b, bool carry = false)
{
    return GenericBinop<Sz, Binop::Add>(a, b, carry);
}

// Returns the result of `a - b`.
template <Size Sz>
constexpr BinopResult<Sz> GenericSubtract(SizeToIntType<Sz> a, SizeToIntType<Sz> b, bool carry = false)
{
    return GenericBinop<Sz, Binop::Subtract>(a, b, carry);
}

} // namespace decoder2
