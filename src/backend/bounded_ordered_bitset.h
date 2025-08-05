#pragma once

#include <bit>
#include <cassert>
#include <cstdint>

// Implements an ordered set of integers backed by a single word. This is
// functionally equivalent to std::set<unsigned_type>, but flat and
// non-allocating. The tradeoff is that this type only supports values in the
// range 0..63. It is intended to be used with small, contiguous enumerations
// like MCU_Interrupt_Source.
//
// Example usage:
//
// BoundedOrderedBitSet<6> s;
// s.Include(5);
// s.Include(0);
// s.Include(2);
// for (auto val : s)
//     printf("%d\n", val);
//
// ==> prints 0, 2, 5

template <size_t Bits>
struct BitsToType;

template <size_t Bits>
    requires(0 < Bits && Bits <= 8)
struct BitsToType<Bits>
{
    using Type = uint8_t;
};

template <size_t Bits>
    requires(8 < Bits && Bits <= 16)
struct BitsToType<Bits>
{
    using Type = uint16_t;
};

template <size_t Bits>
    requires(16 < Bits && Bits <= 32)
struct BitsToType<Bits>
{
    using Type = uint32_t;
};

template <size_t Bits>
    requires(32 < Bits && Bits <= 64)
struct BitsToType<Bits>
{
    using Type = uint64_t;
};

// Bits: number of bits in the set. The range of elements is 0..Bits - 1.
// AccessType: the type to access bits through. May be an enum.
template <size_t Bits, typename AccessType = typename BitsToType<Bits>::Type>
struct BoundedOrderedBitSet
{
public:
    using UnderlyingType = typename BitsToType<Bits>::Type;

    int Size() const
    {
        return std::popcount(m_set);
    }

    void Include(const AccessType& item)
    {
        assert(item < Bits);
        m_set |= static_cast<UnderlyingType>(1 << item);
    }

    void Exclude(const AccessType& item)
    {
        assert(item < Bits);
        m_set &= static_cast<UnderlyingType>(~(1 << item));
    }

    bool Contains(const AccessType& item) const
    {
        assert(item < Bits);
        return m_set & (1 << item);
    }

    class Iterator
    {
    public:
        Iterator& operator++()
        {
            m_state &= m_state - 1;
            return *this;
        }

        AccessType operator*() const
        {
            return static_cast<AccessType>(std::countr_zero(m_state));
        }

        bool operator==(const Iterator& rhs) const
        {
            return m_state == rhs.m_state;
        }

        bool operator!=(const Iterator& rhs) const
        {
            return !(*this == rhs);
        }

    private:
        Iterator(UnderlyingType state)
            : m_state(state)
        {
        }

        UnderlyingType m_state;

        friend BoundedOrderedBitSet<Bits, AccessType>;
    };

    Iterator begin() const
    {
        return Iterator{m_set};
    }

    Iterator end() const
    {
        return Iterator{0};
    }

private:
    UnderlyingType m_set = 0;
};
