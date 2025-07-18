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

#ifndef SCAN_CONFIG_H
#define SCAN_CONFIG_H

#include <cstdint>
#include <limits>

#include <mem/memory/prot_flags.h>

namespace mem
{
    constexpr auto scan_default_block_size = 512 * 1024;
    struct scan_config
    {
        void* start;
        void* end;
        prot_flags flags = prot_flags::NONE;
        std::size_t block_size = 0;

        constexpr scan_config(prot_flags flags_, void* start_ = nullptr,
            void* end_ = reinterpret_cast<void*>(std::numeric_limits<std::uintptr_t>::max()),
            std::size_t block_size_ = scan_default_block_size)
            : start(start_)
            , end(end_)
            , flags(flags_)
            , block_size(block_size_)
        {}

        inline scan_config(std::uintptr_t start_, std::uintptr_t end_ = std::numeric_limits<std::uintptr_t>::max(),
            prot_flags flags_ = prot_flags::RW, std::size_t block_size_ = scan_default_block_size)
            : start(reinterpret_cast<void*>(start_))
            , end(reinterpret_cast<void*>(end_))
            , flags(flags_)
            , block_size(block_size_)
        {}

        constexpr scan_config(void* start_,
            void* end_ = reinterpret_cast<void*>(std::numeric_limits<std::uintptr_t>::max()),
            prot_flags flags_ = prot_flags::RW, std::size_t block_size_ = scan_default_block_size)
            : start(start_)
            , end(end_)
            , flags(flags_)
            , block_size(block_size_)
        {}
    };

    template <typename ScanConfig>
    using is_scan_config = typename std::enable_if<
        std::is_base_of<scan_config, std::decay_t<ScanConfig>>::value>::type;
} // namespace mem

#endif // SCAN_CONFIG_H