#ifndef LOCAL_MEMORY_ACCESSOR_BRICK_H
#define LOCAL_MEMORY_ACCESSOR_BRICK_H

#if defined(_WIN32)
#    if !defined(WIN32_LEAN_AND_MEAN)
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>
#elif defined(__unix__)
#    include <mem/access/proc_maps_utils.h>
#else
#    error Unknown Platform
#endif
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include <mem/access/data_accessor.h>
#include <mem/core/defines.h>
#include <mem/memory/region.h>

namespace mem
{
    class local_memory_accessor : public data_accessor
    {
    public:
        bool read(void* src, void* dst, std::size_t size) const override;

        bool write(void* src, void* dst, std::size_t size) const override;

        bool fill(void* dst, byte value, std::size_t size) const override;

        void* protect_alloc(std::size_t size, prot_flags flags) const override;

        void protect_free(void* addr, std::size_t size) const override;
        bool query_region(void* addr, region_info& region) const override;

        prot_flags protect_query(void* addr) const override;

        bool protect_modify(
            void* addr, std::size_t size, prot_flags flags, prot_flags* old_flags = nullptr) const override;

        void* alloc(std::size_t size) const override;

        void free(void* addr) const override;
    };

    bool local_memory_accessor::read(void* src, void* dst, std::size_t size) const
    {
        if (!src || !dst || size == 0)
            return false;
        std::memcpy(dst, src, size);
        return true;
    }

    bool local_memory_accessor::write(void* src, void* dst, std::size_t size) const
    {
        if (!dst || !src || size == 0)
            return false;
        std::memcpy(src, dst, size);
        return true;
    }

    bool local_memory_accessor::fill(void* dst, byte value, std::size_t size) const
    {
        std::memset(dst, value, size);
        return true;
    }

    void* local_memory_accessor::protect_alloc(std::size_t size, prot_flags flags) const
    {
#if defined(_WIN32)
        return VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, from_prot_flags(flags));
#elif defined(__unix__)
        void* result = mmap(nullptr, size, from_prot_flags(flags), MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (result == MAP_FAILED)
            result = nullptr;
        return result;
#endif
    }

    void local_memory_accessor::protect_free(void* addr, std::size_t size) const
    {
        if (addr)
        {
#if defined(_WIN32)
            (void) size;
            ::VirtualFree(addr, 0, MEM_RELEASE);
#elif defined(__unix__)
            munmap(addr, length);
#endif
        }
    }

    bool local_memory_accessor::query_region(void* addr, region_info& region) const
    {
#if defined(_WIN32)
        MEMORY_BASIC_INFORMATION info;
        if (VirtualQuery(addr, &info, sizeof(info)) != 0)
        {
            region.start = info.BaseAddress;
            region.size = info.RegionSize;
            region.flags = to_prot_flags(info.Protect);
            return true;
        }
        return false;
#elif defined(__unix__)
        internal::region_query query;
        query.address = reinterpret_cast<std::uintptr_t>(addr);

        if (iter_proc_maps(&internal::region_query_callback, &query))
        {
            region = query.region;
            return true;
        }
        return false;
#endif
    }

    prot_flags local_memory_accessor::protect_query(void* addr) const
    {
#if defined(_WIN32)
        region_info region = {0};
        if (query_region(addr, region))
            return region.flags;
        else
            return prot_flags::INVALID;
#elif defined(__unix__)
        internal::prot_query query;
        query.address = reinterpret_cast<std::uintptr_t>(addr);

        if (iter_proc_maps(&internal::prot_query_callback, &query))
        {
            return query.result;
        }

        return prot_flags::INVALID;
#endif
    }

    bool local_memory_accessor::protect_modify(
        void* addr, std::size_t size, prot_flags flags, prot_flags* old_flags) const
    {
        if (flags == prot_flags::INVALID)
            return false;

#if defined(_WIN32)
        DWORD old_protect = 0;
        bool success = ::VirtualProtect(addr, size, from_prot_flags(flags), &old_protect) != FALSE;

        if (old_flags)
            *old_flags = success ? to_prot_flags(old_protect) : prot_flags::INVALID;

        return success;
#elif defined(__unix__)
        if (old_flags)
            *old_flags = protect_query(addr);

        return mprotect(addr, size, from_prot_flags(flags)) == 0;
#endif
    }

    void* local_memory_accessor::alloc(std::size_t size) const
    {
        return std::malloc(size);
    }

    void local_memory_accessor::free(void* addr) const
    {
        if (!addr)
            return;
        std::free(addr);
    }

    MEM_STRONG_INLINE data_accessor& get_default_accessor()
    {
        static local_memory_accessor instance;
        return instance;
    }
} // namespace mem

#endif