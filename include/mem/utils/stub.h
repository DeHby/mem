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

#ifndef MEM_STUB_BRICK_H
#define MEM_STUB_BRICK_H

#include <mem/memory/mem.h>

#if defined(__cpp_lib_invoke) && (__cpp_lib_invoke >= 201411)
#    include <functional>
#else
#    error mem::stub requires std::invoke
#endif

namespace mem
{
    template <typename Func, typename... Args>
    MEM_STRONG_INLINE decltype(auto) stub(mem::pointer address, Args&&... args)
    {
        return std::invoke(address.as<Func>(), std::forward<Args>(args)...);
    }
} // namespace mem

#endif // MEM_STUB_BRICK_H
