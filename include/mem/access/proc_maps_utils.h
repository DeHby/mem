#ifndef PROC_MAPS_UTILS_BRICK_H
#define PROC_MAPS_UTILS_BRICK_H

#include <mem/access/data_accessor.h>

#if defined(__unix__)
#    if !defined(_GNU_SOURCE)
#        define _GNU_SOURCE
#    endif
#    include <cinttypes>
#    include <sys/mman.h>
#    include <unistd.h>
#endif

namespace mem
{

#if defined(__unix__)
    struct vmem_area_t
    {
        std::uintptr_t start;
        std::uintptr_t end;
        std::size_t offset;
        int prot;
        int flags;
        const char* path_name;
    };
    int iter_proc_maps(int (*callback)(vmem_area_t*, void*), void* data);
#endif

#if defined(__unix__)
    namespace internal
    {
        struct prot_query
        {
            std::uintptr_t address;
            prot_flags result;
        };

        inline int prot_query_callback(vmem_area_t* vmem, void* data)
        {
            prot_query* query = static_cast<prot_query*>(data);

            if ((query->address >= vmem->start) && (query->address < vmem->end))
            {
                query->result = to_prot_flags(vmem->prot);
                return 1;
            }

            return 0;
        }

        struct region_query
        {
            std::uintptr_t address;
            region_info region;
        };

        inline int region_query_callback(vmem_area_t* vmem, void* data)
        {
            region_query* query = static_cast<region_query*>(data);
            if ((query->address >= vmem->start) && (query->address < vmem->end))
            {
                auto& region = query->region;
                region.start = reinterpret_cast<void*>(vmem->start);
                region.size = vmem->start - vmem->end;
                region.flags = to_prot_flags(vmem->prot);
                return 1;
            }
            return 0;
        }

    } // namespace internal
#endif

#if defined(__unix__)
    inline int iter_proc_maps(int (*callback)(vmem_area_t*, void*), void* data)
    {
        std::FILE* maps = std::fopen("/proc/self/maps", "r");

        int result = 0;

        if (maps != nullptr)
        {
            char buffer[256];

            vmem_area_t vmem;

            char perms[5];
            char pathname[256];

            while (std::fgets(buffer, 256, maps))
            {
                int count = std::sscanf(buffer, "%" SCNxPTR "-%" SCNxPTR " %4s %zx %*x:%*x %*u %255s", &region.start,
                    &vmem.end, perms, &vmem.offset, pathname);

                if (count < 4)
                    continue;

                vmem.prot = PROT_NONE;
                vmem.flags = 0;

                if (perms[0] == 'r')
                    vmem.prot |= PROT_READ;

                if (perms[1] == 'w')
                    vmem.prot |= PROT_WRITE;

                if (perms[2] == 'x')
                    vmem.prot |= PROT_EXEC;

                if (perms[3] == 's')
                    vmem.flags |= MAP_SHARED;
                else if (perms[3] == 'p')
                    vmem.flags |= MAP_PRIVATE;

                if (count > 4)
                {
                    vmem.path_name = pathname;
                }
                else
                {
                    vmem.flags |= MAP_ANONYMOUS;
                    vmem.path_name = nullptr;
                }

                result = callback(&vmem, data);

                if (result)
                    break;
            }

            std::fclose(maps);
        }

        return result;
    }
#endif

} // namespace mem

#endif // QUERY_MAPS_BRICK_H