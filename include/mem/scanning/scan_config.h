#ifndef SCAN_CONFIG_H
#define SCAN_CONFIG_H

#include <cstdint>
#include <limits>

#include <mem/memory/prot_flags.h>

namespace mem
{
    struct scan_config
    {
        void* start;
        void* end;
        prot_flags flags;
        std::size_t block_size = 4096;

        constexpr scan_config(prot_flags flags_, void* start_ = nullptr,
            void* end_ = reinterpret_cast<void*>(std::numeric_limits<std::uintptr_t>::max()),
            std::size_t block_size_ = 4096)
            : start(start_)
            , end(end_)
            , flags(flags_)
            , block_size(block_size_)
        {}

        constexpr scan_config(void* start_,
            void* end_ = reinterpret_cast<void*>(std::numeric_limits<std::uintptr_t>::max()),
            prot_flags flags_ = prot_flags::RW, std::size_t block_size_ = 4096)
            : start(start_)
            , end(end_)
            , flags(flags_)
            , block_size(block_size_)
        {}
    };
} // namespace mem

#endif // SCAN_CONFIG_H