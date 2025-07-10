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

        constexpr scan_config(prot_flags flags_, void* start_ = nullptr,
            void* end_ = reinterpret_cast<void*>(std::numeric_limits<std::uintptr_t>::max()))
            : start(start_)
            , end(end_)
            , flags(flags_)
        {}

        constexpr scan_config(void* start_, void* end_, prot_flags flags_)
            : start(start_)
            , end(end_)
            , flags(flags_)
        {}
    };
} // namespace mem

#endif // SCAN_CONFIG_H