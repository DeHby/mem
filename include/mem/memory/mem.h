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

#ifndef MEM_BRICK_H
#define MEM_BRICK_H

#include <mem/access/local_memory_accessor.h>
#include <mem/core/defines.h>

#include <cstring>
#include <type_traits>
#include <utility>

namespace mem
{
    class any_pointer;

    class pointer
    {
    private:
        std::uintptr_t value_ {0};

    public:
        constexpr pointer() noexcept;

        constexpr pointer(std::nullptr_t) noexcept;
        constexpr pointer(std::uintptr_t address) noexcept;

        template <typename T>
        pointer(T* address) noexcept;

        template <typename T, typename C>
        pointer(T C::* address) noexcept;

        template <typename T>
        T read_at(std::size_t offset = 0, data_accessor& accessor = get_default_accessor()) const noexcept;

        template <typename T>
        bool write_at(
            const T& value, std::size_t offset = 0, data_accessor& accessor = get_default_accessor()) const noexcept;

        bool read(pointer dst, std::size_t size, data_accessor& accessor = get_default_accessor()) const noexcept;
        bool write(pointer src, std::size_t size, data_accessor& accessor = get_default_accessor()) const noexcept;

        bool fill(byte value, std::size_t size, data_accessor& accessor) const noexcept;
        bool fill(byte value, std::size_t size) const noexcept;

        constexpr pointer add(std::size_t count) const noexcept;
        constexpr pointer sub(std::size_t count) const noexcept;

        constexpr pointer offset(std::ptrdiff_t count) const noexcept;

        constexpr pointer shift(pointer from, pointer to) const noexcept;

        constexpr pointer align_up(std::size_t align) const noexcept;
        constexpr pointer align_down(std::size_t align) const noexcept;

#if defined(MEM_ARCH_X86_64)
        pointer rip(std::size_t offset) const noexcept;
#endif // MEM_ARCH_X86_64

        pointer& deref() const noexcept;

        constexpr pointer operator+(std::size_t count) const noexcept;
        constexpr pointer operator-(std::size_t count) const noexcept;

        constexpr std::ptrdiff_t operator-(pointer rhs) const noexcept;

        MEM_CONSTEXPR_14 pointer& operator+=(std::size_t count) noexcept;
        MEM_CONSTEXPR_14 pointer& operator-=(std::size_t count) noexcept;

        MEM_CONSTEXPR_14 pointer& operator++() noexcept;
        MEM_CONSTEXPR_14 pointer& operator--() noexcept;

        MEM_CONSTEXPR_14 pointer operator++(int) noexcept;
        MEM_CONSTEXPR_14 pointer operator--(int) noexcept;

        constexpr bool operator==(pointer rhs) const noexcept;
        constexpr bool operator!=(pointer rhs) const noexcept;

        constexpr bool operator<(pointer rhs) const noexcept;
        constexpr bool operator>(pointer rhs) const noexcept;

        constexpr bool operator<=(pointer rhs) const noexcept;
        constexpr bool operator>=(pointer rhs) const noexcept;

        constexpr bool operator!() const noexcept;

        constexpr explicit operator bool() const noexcept;

        template <typename T = pointer>
        typename std::add_lvalue_reference<T>::type at(std::size_t offset) const noexcept;

        template <typename T>
        constexpr typename std::enable_if<std::is_integral<T>::value, T>::type as() const noexcept;

        template <typename T>
        typename std::enable_if<std::is_pointer<T>::value, T>::type as() const noexcept;

        template <typename T>
        typename std::enable_if<std::is_member_pointer<T>::value, T>::type as() const noexcept;

        template <typename T>
        typename std::enable_if<std::is_lvalue_reference<T>::value, T>::type as() const noexcept;

        template <typename T>
        typename std::enable_if<std::is_array<T>::value, typename std::add_lvalue_reference<T>::type>::type
        as() const noexcept;

        template <typename T>
        typename std::enable_if<!std::is_reference<T>::value, typename std::add_lvalue_reference<T>::type>::type
        rcast() & noexcept;

        template <typename Func>
        constexpr pointer and_then(Func&& func) const;

        template <typename Func>
        constexpr pointer or_else(Func&& func) const;

        constexpr any_pointer any() const noexcept;
    };

    static_assert((sizeof(pointer) == sizeof(void*)) && (alignof(pointer) == alignof(void*)), "Hmm...");

    class any_pointer
    {
    private:
        std::uintptr_t value_ {0};

    public:
        constexpr any_pointer(std::uintptr_t value) noexcept;

        constexpr operator std::uintptr_t() const noexcept;

        template <typename T>
        operator T*() const noexcept;
    };

    template <typename T>
    typename std::add_lvalue_reference<T>::type field(pointer base, std::ptrdiff_t offset = 0) noexcept;

    template <typename F>
    typename std::add_lvalue_reference<F>::type vfunc(
        pointer inst, std::size_t index, std::ptrdiff_t table = 0) noexcept;

    template <typename To, typename From>
    To bit_cast(const From& src) noexcept;

    MEM_STRONG_INLINE constexpr pointer::pointer() noexcept = default;

    MEM_STRONG_INLINE constexpr pointer::pointer(std::nullptr_t) noexcept
        : value_(0)
    {}

    MEM_STRONG_INLINE constexpr pointer::pointer(std::uintptr_t address) noexcept
        : value_(address)
    {}

    template <typename T>
    MEM_STRONG_INLINE pointer::pointer(T* address) noexcept
        : value_(reinterpret_cast<std::uintptr_t>(address))
    {}

    template <typename T, typename C>
    MEM_STRONG_INLINE pointer::pointer(T C::* address) noexcept
        : value_(bit_cast<std::uintptr_t>(address))
    {}

    template <typename T>
    MEM_STRONG_INLINE T pointer::read_at(std::size_t offset, data_accessor& accessor) const noexcept
    {
        T value {};
        accessor.read(value_ + offset, &value, sizeof(T));
        return value;
    }

    template <typename T>
    MEM_STRONG_INLINE bool pointer::write_at(const T& value, std::size_t offset, data_accessor& accessor) const noexcept
    {
        return accessor.write(value_ + offset, &value, sizeof(T));
    }

    MEM_STRONG_INLINE bool pointer::read(pointer dst, std::size_t size, data_accessor& accessor) const noexcept
    {
        return accessor.read(reinterpret_cast<void*>(value_), dst.any(), size);
    }

    MEM_STRONG_INLINE bool pointer::write(pointer src, std::size_t size, data_accessor& accessor) const noexcept
    {
        return accessor.write(reinterpret_cast<void*>(value_), src.any(), size);
    }

    MEM_STRONG_INLINE bool pointer::fill(byte value, std::size_t size, data_accessor& accessor) const noexcept
    {
        return accessor.fill(reinterpret_cast<void*>(value_), value, size);
    }

    MEM_STRONG_INLINE bool pointer::fill(byte value, std::size_t size) const noexcept
    {
        return fill(value, size, get_default_accessor());
    }

    MEM_STRONG_INLINE constexpr pointer pointer::add(std::size_t count) const noexcept
    {
        return value_ + count;
    }

    MEM_STRONG_INLINE constexpr pointer pointer::sub(std::size_t count) const noexcept
    {
        return value_ - count;
    }

    MEM_STRONG_INLINE constexpr pointer pointer::offset(std::ptrdiff_t count) const noexcept
    {
        return static_cast<std::uintptr_t>(static_cast<std::intptr_t>(value_) + count);
    }

    MEM_STRONG_INLINE constexpr pointer pointer::shift(pointer from, pointer to) const noexcept
    {
        return (value_ - from.value_) + to.value_;
    }

    MEM_STRONG_INLINE constexpr pointer pointer::align_up(std::size_t align) const noexcept
    {
        return (value_ + align - 1) / align * align;
    }

    MEM_STRONG_INLINE constexpr pointer pointer::align_down(std::size_t align) const noexcept
    {
        return value_ - (value_ % align);
    }

#if defined(MEM_ARCH_X86_64)
    MEM_STRONG_INLINE pointer pointer::rip(std::size_t offset) const noexcept
    {
        return static_cast<std::uintptr_t>(
            static_cast<std::intptr_t>(value_ + offset) + *reinterpret_cast<const std::int32_t*>(value_));
    }
#endif // MEM_ARCH_X86_64

    MEM_STRONG_INLINE pointer& pointer::deref() const noexcept
    {
        return *reinterpret_cast<pointer*>(value_);
    }

    MEM_STRONG_INLINE constexpr pointer pointer::operator+(std::size_t count) const noexcept
    {
        return value_ + count;
    }

    MEM_STRONG_INLINE constexpr pointer pointer::operator-(std::size_t count) const noexcept
    {
        return value_ - count;
    }

    MEM_STRONG_INLINE constexpr std::ptrdiff_t pointer::operator-(pointer rhs) const noexcept
    {
        return static_cast<std::ptrdiff_t>(static_cast<std::intptr_t>(value_) - static_cast<std::intptr_t>(rhs.value_));
    }

    MEM_STRONG_INLINE MEM_CONSTEXPR_14 pointer& pointer::operator+=(std::size_t count) noexcept
    {
        value_ += count;

        return *this;
    }

    MEM_STRONG_INLINE MEM_CONSTEXPR_14 pointer& pointer::operator-=(std::size_t count) noexcept
    {
        value_ -= count;

        return *this;
    }

    MEM_STRONG_INLINE MEM_CONSTEXPR_14 pointer& pointer::operator++() noexcept
    {
        ++value_;

        return *this;
    }

    MEM_STRONG_INLINE MEM_CONSTEXPR_14 pointer& pointer::operator--() noexcept
    {
        --value_;

        return *this;
    }

    MEM_STRONG_INLINE MEM_CONSTEXPR_14 pointer pointer::operator++(int) noexcept
    {
        pointer result = *this;

        ++value_;

        return result;
    }

    MEM_STRONG_INLINE MEM_CONSTEXPR_14 pointer pointer::operator--(int) noexcept
    {
        pointer result = *this;

        --value_;

        return result;
    }

    MEM_STRONG_INLINE constexpr bool pointer::operator==(pointer rhs) const noexcept
    {
        return value_ == rhs.value_;
    }

    MEM_STRONG_INLINE constexpr bool pointer::operator!=(pointer rhs) const noexcept
    {
        return value_ != rhs.value_;
    }

    MEM_STRONG_INLINE constexpr bool pointer::operator<(pointer rhs) const noexcept
    {
        return value_ < rhs.value_;
    }

    MEM_STRONG_INLINE constexpr bool pointer::operator>(pointer rhs) const noexcept
    {
        return value_ > rhs.value_;
    }

    MEM_STRONG_INLINE constexpr bool pointer::operator<=(pointer rhs) const noexcept
    {
        return value_ <= rhs.value_;
    }

    MEM_STRONG_INLINE constexpr bool pointer::operator>=(pointer rhs) const noexcept
    {
        return value_ >= rhs.value_;
    }

    MEM_STRONG_INLINE constexpr bool pointer::operator!() const noexcept
    {
        return !value_;
    }

    MEM_STRONG_INLINE constexpr pointer::operator bool() const noexcept
    {
        return value_ != 0;
    }

    template <typename T>
    MEM_STRONG_INLINE constexpr typename std::enable_if<std::is_integral<T>::value, T>::type
    pointer::as() const noexcept
    {
        static_assert(
            std::is_same<typename std::make_unsigned<T>::type, std::uintptr_t>::value, "Invalid Integer Type");

        return static_cast<T>(value_);
    }

    template <typename T>
    MEM_STRONG_INLINE typename std::add_lvalue_reference<T>::type pointer::at(std::size_t offset) const noexcept
    {
        return *reinterpret_cast<typename std::add_pointer<T>::type>(value_ + offset);
    }

    template <typename T>
    MEM_STRONG_INLINE typename std::enable_if<std::is_pointer<T>::value, T>::type pointer::as() const noexcept
    {
        return reinterpret_cast<T>(value_);
    }

    template <typename T>
    MEM_STRONG_INLINE typename std::enable_if<std::is_member_pointer<T>::value, T>::type pointer::as() const noexcept
    {
        return bit_cast<T>(value_);
    }

    template <typename T>
    MEM_STRONG_INLINE typename std::enable_if<std::is_lvalue_reference<T>::value, T>::type pointer::as() const noexcept
    {
        return *reinterpret_cast<typename std::add_pointer<T>::type>(value_);
    }

    template <typename T>
    MEM_STRONG_INLINE
        typename std::enable_if<std::is_array<T>::value, typename std::add_lvalue_reference<T>::type>::type
        pointer::as() const noexcept
    {
        return *reinterpret_cast<typename std::add_pointer<T>::type>(value_);
    }

    template <typename T>
    MEM_STRONG_INLINE
        typename std::enable_if<!std::is_reference<T>::value, typename std::add_lvalue_reference<T>::type>::type
        pointer::rcast() & noexcept
    {
        static_assert(sizeof(T) == sizeof(pointer), "That's no pointer. It's a space station.");

        return *reinterpret_cast<typename std::add_pointer<T>::type>(this);
    }

    template <typename Func>
    MEM_STRONG_INLINE constexpr pointer pointer::and_then(Func&& func) const
    {
        return value_ ? std::forward<Func>(func)(*this) : nullptr;
    }

    template <typename Func>
    MEM_STRONG_INLINE constexpr pointer pointer::or_else(Func&& func) const
    {
        return value_ ? *this : std::forward<Func>(func)();
    }

    MEM_STRONG_INLINE constexpr any_pointer pointer::any() const noexcept
    {
        return any_pointer(value_);
    }

    MEM_STRONG_INLINE constexpr any_pointer::any_pointer(std::uintptr_t value) noexcept
        : value_(value)
    {}

    MEM_STRONG_INLINE constexpr any_pointer::operator std::uintptr_t() const noexcept
    {
        return value_;
    }

    template <typename T>
    MEM_STRONG_INLINE any_pointer::operator T*() const noexcept
    {
        return reinterpret_cast<T*>(value_);
    }

    template <typename T>
    MEM_STRONG_INLINE typename std::add_lvalue_reference<T>::type field(pointer base, std::ptrdiff_t offset) noexcept
    {
        return base.at<T>(offset);
    }

    template <typename F>
    MEM_STRONG_INLINE typename std::add_lvalue_reference<F>::type vfunc(
        pointer inst, std::size_t index, std::ptrdiff_t table) noexcept
    {
        return inst.as<pointer**>()[table][index].rcast<F>();
    }

    template <typename To, typename From>
    MEM_STRONG_INLINE To bit_cast(const From& src) noexcept
    {
        static_assert(sizeof(To) == sizeof(From), "sizeof(To) != sizeof(From)");
        static_assert(std::is_trivially_copyable<From>::value, "From is not trivially copyable");
        static_assert(std::is_trivially_copyable<To>::value, "To is not trivially copyable");

        typename std::aligned_storage<sizeof(To), alignof(To)>::type dst;
        std::memcpy(&dst, &src, sizeof(To));
        return reinterpret_cast<To&>(dst);
    }
} // namespace mem
#endif // !MEM_BRICK_H
