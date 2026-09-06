/*
 * Copyright (C) 2024-2026 J.C. Moyer
 *
 * This file is part of Nuked-SC55.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#pragma once

#include <atomic>
#include <cassert>
#include <memory>
#include <span>

// This type has reference semantics.
class GenericBuffer
{
public:
    GenericBuffer() = default;

    ~GenericBuffer()
    {
        Free();
    }

    GenericBuffer(const GenericBuffer&)            = delete;
    GenericBuffer& operator=(const GenericBuffer&) = delete;

    GenericBuffer(GenericBuffer&&)            = delete;
    GenericBuffer& operator=(GenericBuffer&&) = delete;

    bool Init(size_t size_bytes)
    {
        Free();

        size_t alloc_size = 64 + size_bytes;

        m_alloc_base = malloc(alloc_size);
        if (!m_alloc_base)
        {
            return false;
        }

        m_buffer      = m_alloc_base;
        m_buffer_size = size_bytes;
        if (!std::align(64, size_bytes, m_buffer, alloc_size))
        {
            Free();
            return false;
        }

        return true;
    }

    void Free()
    {
        if (m_alloc_base)
        {
            free(m_alloc_base);
        }
        m_buffer      = nullptr;
        m_buffer_size = 0;
        m_alloc_base  = nullptr;
    }

    void* DataFirst()
    {
        return std::assume_aligned<64>(m_buffer);
    }

    const void* DataFirst() const
    {
        return std::assume_aligned<64>(m_buffer);
    }

    void* DataLast()
    {
        return (uint8_t*)DataFirst() + m_buffer_size;
    }

    const void* DataLast() const
    {
        return (uint8_t*)DataFirst() + m_buffer_size;
    }

    size_t GetByteLength() const
    {
        return m_buffer_size;
    }

private:
    void*  m_buffer      = nullptr;
    size_t m_buffer_size = 0;
    void*  m_alloc_base  = nullptr;
};

class RingbufferView
{
public:
    RingbufferView() = default;

    explicit RingbufferView(GenericBuffer& buffer)
        : m_buffer((uint8_t*)buffer.DataFirst(), (uint8_t*)buffer.DataLast())
    {
        m_read_head  = 0;
        m_write_head = 0;
    }

    RingbufferView(const RingbufferView& rhs)
    {
        m_read_head  = rhs.m_read_head.load();
        m_write_head = rhs.m_write_head.load();
        m_buffer     = rhs.m_buffer;
    }

    RingbufferView& operator=(const RingbufferView& rhs)
    {
        m_read_head  = rhs.m_read_head.load();
        m_write_head = rhs.m_write_head.load();
        m_buffer     = rhs.m_buffer;
        return *this;
    }

    RingbufferView(RingbufferView&& rhs) noexcept
    {
        m_read_head  = rhs.m_read_head.load();
        m_write_head = rhs.m_write_head.load();
        m_buffer     = rhs.m_buffer;
    }

    RingbufferView& operator=(RingbufferView&& rhs) noexcept
    {
        m_read_head  = rhs.m_read_head.load();
        m_write_head = rhs.m_write_head.load();
        m_buffer     = rhs.m_buffer;
        return *this;
    }

    template <typename ElemT>
    void UncheckedWriteOne(const ElemT& value)
    {
        memcpy(GetWritePtr(), &value, sizeof(ElemT));
        m_write_head = Mask2(m_write_head + sizeof(ElemT));
    }

    template <typename ElemT>
    void UncheckedReadOne(ElemT& dest)
    {
        memcpy(&dest, GetReadPtr(), sizeof(ElemT));
        m_read_head = Mask2(m_read_head + sizeof(ElemT));
    }

    template <typename ElemT>
    std::span<ElemT> UncheckedPrepareWrite(size_t count)
    {
        // count must be an integer divisor of the buffer size
        assert((m_buffer.size() / sizeof(ElemT)) % count == 0);
        // write must start at the end of a prior `count`-long write
        assert((m_write_head / sizeof(ElemT)) % count == 0);
        // must have space for `count` elements
        assert(GetWritableElements<ElemT>() >= count);
        return {(ElemT*)GetWritePtr(), count};
    }

    template <typename ElemT>
    void UncheckedFinishWrite(size_t count)
    {
        assert(m_write_head % count == 0);
        m_write_head = Mask2(m_write_head + count * sizeof(ElemT));
    }

    template <typename ElemT>
    std::span<ElemT> UncheckedPrepareRead(size_t count)
    {
        // count must be an integer divisor of the buffer size
        assert((m_buffer.size() / sizeof(ElemT)) % count == 0);
        // read must start at the end of a prior `count`-long read
        assert((m_read_head / sizeof(ElemT)) % count == 0);
        // must have `count` elements
        assert(GetReadableElements<ElemT>() >= count);
        return {(ElemT*)GetReadPtr(), count};
    }

    template <typename ElemT>
    void UncheckedFinishRead(size_t count)
    {
        assert(m_read_head % count == 0);
        m_read_head = Mask2(m_read_head + count * sizeof(ElemT));
    }

    size_t GetReadableBytes() const
    {
        return Mask(m_write_head - m_read_head);
    }

    size_t GetWritableBytes() const
    {
        return m_buffer.size() - GetReadableBytes();
    }

    template <typename ElemT>
    size_t GetReadableElements() const
    {
        return GetReadableBytes() / sizeof(ElemT);
    }

    template <typename ElemT>
    size_t GetWritableElements() const
    {
        return GetWritableBytes() / sizeof(ElemT);
    }

private:
    uint8_t* GetWritePtr()
    {
        return m_buffer.data() + Mask(m_write_head);
    }

    const uint8_t* GetReadPtr() const
    {
        return m_buffer.data() + Mask(m_read_head);
    }

    size_t Mask(size_t index) const
    {
        return index & (m_buffer.size() - 1);
    }

    size_t Mask2(size_t index) const
    {
        return index & (2 * m_buffer.size() - 1);
    }

private:
    std::span<uint8_t>  m_buffer;
    std::atomic<size_t> m_read_head  = 0;
    std::atomic<size_t> m_write_head = 0;
};

