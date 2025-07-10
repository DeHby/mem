/*
    Copyright 2018 Brick

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

#ifndef MEM_PROTECT_BRICK_H
#define MEM_PROTECT_BRICK_H

#include <mem/memory/region.h>
#include <mem/memory/prot_flags.h>
#include <mem/access/local_memory_accessor.h>

#include <cstdio>

namespace mem
{
    std::size_t page_size();

    void* protect_alloc(std::size_t length, prot_flags flags, data_accessor& accessor = get_default_accessor());
    void protect_free(void* memory, std::size_t length, data_accessor& accessor = get_default_accessor());

    prot_flags protect_query(void* memory, data_accessor& accessor = get_default_accessor());

    bool protect_modify(void* memory, std::size_t length, prot_flags flags, prot_flags* old_flags = nullptr,
        data_accessor& accessor = get_default_accessor());

    class protect : public region
    {
    private:
        prot_flags old_flags_ {prot_flags::INVALID};
        bool success_ {false};
        data_accessor* accessor_ {nullptr};

    public:
        protect(region range, data_accessor& accessor = get_default_accessor(), prot_flags flags = prot_flags::RWX);
        ~protect();

        protect(protect&& rhs) noexcept;
        protect(const protect&) = delete;

        explicit operator bool() const noexcept;
        prot_flags release() noexcept;
    };

    inline std::size_t page_size()
    {
#if defined(_WIN32)
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return static_cast<std::size_t>(si.dwPageSize);
#elif defined(__unix__)
        return static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
#endif
    }

    inline void* protect_alloc(std::size_t length, prot_flags flags, mem::data_accessor& accessor)
    {
        return accessor.protect_alloc(length, flags);
    }

    inline void protect_free(void* memory, std::size_t length, data_accessor& accessor)
    {
        return accessor.protect_free(memory, length);
    }

    inline prot_flags protect_query(void* memory, data_accessor& accessor)
    {
        return accessor.protect_query(memory);
    }

    inline bool protect_modify(
        void* memory, std::size_t length, prot_flags flags, prot_flags* old_flags, data_accessor& accessor)
    {
        return accessor.protect_modify(memory, length, flags, old_flags);
    }

    MEM_STRONG_INLINE protect::protect(region range, data_accessor& accessor, prot_flags flags)
        : region(range)
        , accessor_(&accessor)
        , old_flags_(prot_flags::INVALID)
        , success_(protect_modify(start.as<void*>(), size, flags, &old_flags_, *accessor_))
    {}

    MEM_STRONG_INLINE protect::~protect()
    {
        if (success_)
        {
            protect_modify(start.as<void*>(), size, old_flags_, nullptr, *accessor_);
        }
    }

    MEM_STRONG_INLINE protect::protect(protect&& rhs) noexcept
        : region(rhs)
        , accessor_(rhs.accessor_)
        , old_flags_(rhs.old_flags_)
        , success_(rhs.success_)
    {
        rhs.old_flags_ = prot_flags::INVALID;
        rhs.success_ = false;
    }

    MEM_STRONG_INLINE protect::operator bool() const noexcept
    {
        return success_;
    }

    MEM_STRONG_INLINE prot_flags protect::release() noexcept
    {
        success_ = false;

        return old_flags_;
    }
} // namespace mem

#endif // MEM_PROTECT_BRICK_H
