/*
 * Copyright (C) 2021, 2024 nukeykt
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include "audio.h"
#include "math_util.h"
#include <cassert>
#include <memory>
#include <span>
#include <atomic>

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

    void* DataLast()
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

