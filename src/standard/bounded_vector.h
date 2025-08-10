#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <utility>

// Provides inline storage for up to N elements of T. This container does not
// reallocate so pointer stability is guaranteed as long as elements are only
// appended.
template <typename T, size_t N>
class BoundedVector
{
public:
    constexpr BoundedVector() = default;

    constexpr ~BoundedVector()
    {
        for (size_t i = 0; i < Count(); ++i)
        {
            UncheckedAt(i).~T();
        }
    }

    // Copy/move not necessary right now
    BoundedVector(const BoundedVector&)            = delete;
    BoundedVector& operator=(const BoundedVector&) = delete;
    BoundedVector(BoundedVector&&)                 = delete;
    BoundedVector& operator=(BoundedVector&&)      = delete;

    template <typename... Args>
    constexpr T& EmplaceBack(Args&&... args)
    {
        if (IsFull()) [[unlikely]]
        {
            fprintf(stderr, "BoundedVector EmplaceBack when full\n");
            exit(1);
        }
        T* ptr = new (&UncheckedAt(m_elem_count)) T(std::forward<Args>(args)...);
        ++m_elem_count;
        return *ptr;
    }

    constexpr void PopBack()
    {
        if (IsEmpty()) [[unlikely]]
        {
            fprintf(stderr, "BoundedVector PopBack when empty\n");
            exit(1);
        }
        --m_elem_count;
        UncheckedAt(m_elem_count).~T();
    }

    constexpr size_t Count() const
    {
        return m_elem_count;
    }

    constexpr bool IsEmpty() const
    {
        return Count() == 0;
    }

    constexpr bool IsFull() const
    {
        return Count() == N;
    }

    constexpr T& operator[](size_t i)
    {
        if (i >= m_elem_count) [[unlikely]]
        {
            fprintf(stderr, "BoundedVector index out of range %zu\n", i);
            exit(1);
        }
        return UncheckedAt(i);
    }

    constexpr T& UncheckedAt(size_t i)
    {
        return *reinterpret_cast<T*>(&m_storage[i * sizeof(T)]);
    }

    constexpr const T& UncheckedAt(size_t i) const
    {
        return *reinterpret_cast<const T*>(&m_storage[i * sizeof(T)]);
    }

    constexpr T* begin()
    {
        return &UncheckedAt(0);
    }

    constexpr const T* begin() const
    {
        return &UncheckedAt(0);
    }

    constexpr T* end()
    {
        return &UncheckedAt(m_elem_count);
    }

    constexpr const T* end() const
    {
        return &UncheckedAt(m_elem_count);
    }

private:
    alignas(T) std::byte m_storage[sizeof(T) * N];
    size_t m_elem_count = 0;
};
