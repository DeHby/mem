/*
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

#ifndef REMOTE_MEMORY_ACCESSOR_BRICK_H
#define REMOTE_MEMORY_ACCESSOR_BRICK_H

#ifndef _WIN32
#    error "Only the Windows platform is supported"
#endif

#if defined(_WIN32)
#    if !defined(WIN32_LEAN_AND_MEAN)
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>
#endif

#include <mem/access/data_accessor.h>
#include <mem/memory/prot_flags.h>

#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace mem
{
    class remote_memory_accessor : public data_accessor
    {
    protected:
        HANDLE process_handle_;
        bool owns_handle_;

        constexpr remote_memory_accessor(HANDLE handle, bool owns = false);

    public:
        ~remote_memory_accessor();

        remote_memory_accessor(const remote_memory_accessor&) = delete;
        remote_memory_accessor& operator=(const remote_memory_accessor&) = delete;

        remote_memory_accessor(remote_memory_accessor&& other) noexcept;

        remote_memory_accessor& operator=(remote_memory_accessor&& other) noexcept;

        static remote_memory_accessor create(DWORD pid);

        static remote_memory_accessor create(HANDLE handle);

        HANDLE handle() const;

        bool read(void* src, void* dst, std::size_t size) const override;

        bool write(void* src, void* dst, std::size_t size) const override;

        void* protect_alloc(std::size_t size, prot_flags flags) const override;

        void protect_free(void* addr, std::size_t size) const override;

        bool query_region(void* addr, region_info& region) const override;

        prot_flags protect_query(void* addr) const override;

        bool protect_modify(void* addr, std::size_t size, prot_flags flags, prot_flags* old_flags) const override;

        void* alloc(std::size_t size) const override;

        void free(void* addr) const override;
    };

    MEM_STRONG_INLINE constexpr remote_memory_accessor::remote_memory_accessor(HANDLE handle, bool owns)
        : process_handle_(handle)
        , owns_handle_(owns)
    {}

    MEM_STRONG_INLINE remote_memory_accessor::~remote_memory_accessor()
    {
        if (owns_handle_ && process_handle_ && process_handle_ != INVALID_HANDLE_VALUE)
            CloseHandle(process_handle_);
    }

    MEM_STRONG_INLINE remote_memory_accessor::remote_memory_accessor(remote_memory_accessor&& other) noexcept
        : process_handle_(other.process_handle_)
        , owns_handle_(other.owns_handle_)
    {
        other.process_handle_ = nullptr;
        other.owns_handle_ = false;
    }

    MEM_STRONG_INLINE remote_memory_accessor& remote_memory_accessor::operator=(remote_memory_accessor&& other) noexcept
    {
        if (this != &other)
        {
            if (owns_handle_ && process_handle_ && process_handle_ != INVALID_HANDLE_VALUE)
                ::CloseHandle(process_handle_);

            process_handle_ = other.process_handle_;
            owns_handle_ = other.owns_handle_;

            other.process_handle_ = nullptr;
            other.owns_handle_ = false;
        }
        return *this;
    }

    MEM_STRONG_INLINE remote_memory_accessor remote_memory_accessor::create(DWORD pid)
    {
        HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (h == NULL)
            throw std::runtime_error("Failed to open process handle");
        return remote_memory_accessor(h, true);
    }

    MEM_STRONG_INLINE remote_memory_accessor remote_memory_accessor::create(HANDLE handle)
    {
        if (!handle || handle == INVALID_HANDLE_VALUE)
            throw std::invalid_argument("Invalid process handle");
        return remote_memory_accessor(handle);
    }

    MEM_STRONG_INLINE HANDLE remote_memory_accessor::handle() const
    {
        return process_handle_;
    }

    MEM_STRONG_INLINE bool remote_memory_accessor::read(void* src, void* dst, std::size_t size) const
    {
        if (!src || !dst || size == 0)
            return false;
        SIZE_T bytesRead = 0;
        return ::ReadProcessMemory(process_handle_, src, dst, size, &bytesRead) && bytesRead == size;
    }

    MEM_STRONG_INLINE bool remote_memory_accessor::write(void* dst, void* src, std::size_t size) const
    {
        if (!dst || !src || size == 0)
            return false;
        SIZE_T bytesWritten = 0;
        return ::WriteProcessMemory(process_handle_, dst, src, size, &bytesWritten) && bytesWritten == size;
    }

    MEM_STRONG_INLINE void* remote_memory_accessor::protect_alloc(std::size_t size, prot_flags flags) const
    {
        return ::VirtualAllocEx(process_handle_, NULL, size, MEM_COMMIT | MEM_RESERVE, from_prot_flags(flags));
    }

    MEM_STRONG_INLINE void remote_memory_accessor::protect_free(void* addr, std::size_t size) const
    {
        ::VirtualFreeEx(process_handle_, addr, 0, MEM_RELEASE);
    }

    MEM_STRONG_INLINE bool remote_memory_accessor::query_region(void* addr, region_info& region) const
    {
        MEMORY_BASIC_INFORMATION info;
        if (::VirtualQueryEx(process_handle_, addr, &info, sizeof(info)) != 0)
        {
            region.start = info.BaseAddress;
            region.size = info.RegionSize;
            region.flags = to_prot_flags(info.Protect);
            return true;
        }
        return false;
    }

    MEM_STRONG_INLINE prot_flags remote_memory_accessor::protect_query(void* addr) const
    {
        region_info info = {};
        if (query_region(addr, info))
            return info.flags;
        else
            return prot_flags::INVALID;
    }

    MEM_STRONG_INLINE bool remote_memory_accessor::protect_modify(
        void* addr, std::size_t size, prot_flags flags, prot_flags* old_flags) const
    {
        if (flags == prot_flags::INVALID)
            return false;
        DWORD old_protect = 0;
        bool success = ::VirtualProtectEx(process_handle_, addr, size, from_prot_flags(flags), &old_protect) != FALSE;
        if (old_flags)
            *old_flags = success ? to_prot_flags(old_protect) : prot_flags::INVALID;
        return success;
    }

    MEM_STRONG_INLINE void* remote_memory_accessor::alloc(std::size_t size) const
    {
        return protect_alloc(size, mem::prot_flags::RW);
    }

    MEM_STRONG_INLINE void remote_memory_accessor::free(void* addr) const
    {
        return protect_free(addr, 0);
    }

    class current_process_accessor : public remote_memory_accessor
    {
    public:
        MEM_STRONG_INLINE current_process_accessor()
            : remote_memory_accessor(::GetCurrentProcess(), false)
        {}
    };

} // namespace mem

#endif