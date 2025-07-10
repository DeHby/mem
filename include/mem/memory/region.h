
#ifndef REGION_BRICK_H
#define REGION_BRICK_H

#include <mem/memory/common.h>
#include <mem/memory/mem.h>
#include <mem/memory/prot_flags.h>

namespace mem
{
    class region
    {
    public:
        pointer start {nullptr};
        std::size_t size {0};
        prot_flags flags {prot_flags::NONE};

        constexpr region() noexcept;

        region(const region_info& info);

        constexpr region(pointer start, std::size_t size, prot_flags flags = prot_flags::NONE) noexcept;

        constexpr bool contains(region rhs) const noexcept;

        constexpr bool contains(pointer address) const noexcept;
        constexpr bool contains(pointer start, std::size_t size) const noexcept;

        template <typename T>
        constexpr bool contains(pointer address) const noexcept;

        constexpr bool operator==(region rhs) const noexcept;
        constexpr bool operator!=(region rhs) const noexcept;

        constexpr region sub_region(pointer address) const noexcept;
    };

    MEM_STRONG_INLINE constexpr region::region() noexcept = default;

    MEM_STRONG_INLINE region::region(const region_info& info)
        : start(info.start)
        , size(info.size)
        , flags(info.flags)
    {}

    MEM_STRONG_INLINE constexpr region::region(pointer start_, std::size_t size_, prot_flags flags_) noexcept
        : start(start_)
        , size(size_)
        , flags(flags_)
    {}

    MEM_STRONG_INLINE constexpr bool region::contains(region rhs) const noexcept
    {
        return (rhs.start >= start) && ((rhs.start + rhs.size) <= (start + size));
    }

    MEM_STRONG_INLINE constexpr bool region::contains(pointer address) const noexcept
    {
        return (address >= start) && (address < (start + size));
    }

    MEM_STRONG_INLINE constexpr bool region::contains(pointer start_, std::size_t size_) const noexcept
    {
        return (start_ >= start) && ((start_ + size_) <= (start + size));
    }

    template <typename T>
    MEM_STRONG_INLINE constexpr bool region::contains(pointer address) const noexcept
    {
        return (address >= start) && ((address + sizeof(T)) <= (start + size));
    }

    MEM_STRONG_INLINE constexpr bool region::operator==(region rhs) const noexcept
    {
        return (start == rhs.start) && (size == rhs.size) && (flags != rhs.flags);
    }

    MEM_STRONG_INLINE constexpr bool region::operator!=(region rhs) const noexcept
    {
        return (start != rhs.start) || (size != rhs.size) || (flags != rhs.flags);
    }

    MEM_STRONG_INLINE constexpr region region::sub_region(pointer address) const noexcept
    {
        return region(address, size - static_cast<std::size_t>(address - start), flags);
    }
} // namespace mem

#endif