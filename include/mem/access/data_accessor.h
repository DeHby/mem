﻿/*
    Copyright 2025 DeHby

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software
    and associated documentation files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge, publish, distribute,
    sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
    BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef DATA_ACCESSOR_BRICK_H
#define DATA_ACCESSOR_BRICK_H

#include <mem/memory/common.h>
#include <mem/memory/prot_flags.h>

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace mem
{
    class data_accessor;

    template <typename Accessor>
    using is_accessor = typename std::enable_if<std::is_base_of<data_accessor, std::decay_t<Accessor>>::value>::type;

    class data_accessor
    {
    public:
        virtual bool read(void* src, void* dst, std::size_t size) const = 0;
        virtual bool write(void* src, void* dst, std::size_t size) const = 0;
        virtual bool fill(void* dst, byte value, std::size_t size) const;

        virtual void* protect_alloc(std::size_t size, prot_flags flags) const = 0;
        virtual void protect_free(void* addr, std::size_t size) const = 0;

        virtual bool query_region(void* addr, region_info& region) const = 0;
        virtual prot_flags protect_query(void* addr) const = 0;
        virtual bool protect_modify(
            void* addr, std::size_t size, prot_flags flags, prot_flags* old_flags = nullptr) const = 0;

        virtual void* alloc(std::size_t size) const = 0;
        virtual void free(void* addr) const = 0;

        virtual ~data_accessor() = default;
    };

    MEM_STRONG_INLINE bool data_accessor::fill(void* dst, byte value, std::size_t size) const
    {
        if (!dst || size == 0)
            return false;

        constexpr size_t chunk_size = 4096;
        uint8_t buffer[chunk_size];
        std::memset(buffer, value, chunk_size);

        std::size_t remaining = size;
        std::uintptr_t cur_addr = reinterpret_cast<std::uintptr_t>(dst);

        while (remaining > 0)
        {
            size_t write_size = std::min(remaining, chunk_size);
            if (!this->write(reinterpret_cast<void*>(cur_addr), buffer, write_size))
                return false;
            cur_addr += write_size;
            remaining -= write_size;
        }
        return true;
    };
} // namespace mem

#endif // DATA_ASSESSOR_BRICK_H