#ifndef MEM_DEFINED_BRICK_H
#define MEM_DEFINED_BRICK_H

#include <mem/memory/prot_flags.h>

#include <stdint.h>

namespace mem
{
    struct region_info
    {
        void* start;
        std::size_t size;
        prot_flags flags;
    };

} // namespace mem
#endif