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

#ifndef MEMORY_SCANNER_H
#define MEMORY_SCANNER_H

#include <mem/access/remote_memory_accessor.h>

#include <mem/scanning/boyer_moore_scanner.h>
#include <mem/scanning/simd_scanner.h>

#include <mem/scanning/pattern.h>
#include <mem/scanning/scan_config.h>

#include <functional>
#include <vector>

namespace mem
{
    class memory_scanner
    {
    private:
        data_accessor& accessor_;

    public:
        constexpr memory_scanner(data_accessor& accessor);

        template <typename Scanner = boyer_moore_scanner, typename Config, typename = is_scanner<Scanner>,
            typename = is_scan_config<Config>>
        std::vector<pointer> scan(Scanner&& scanner, Config&& config) const;

        template <typename Scanner = boyer_moore_scanner, typename... Args, typename Config,
            typename = is_scanner<Scanner>, typename = is_scan_config<Config>>
        auto scan(Config&& config, Args&&... args) const;

        template <typename Scanner = boyer_moore_scanner, typename... Args>
        constexpr static auto scan_default(Args&&... args);

        constexpr const data_accessor& get_accessor() const;
    };

    constexpr memory_scanner::memory_scanner(data_accessor& accessor)
        : accessor_(accessor)
    {}

    template <typename Scanner, typename Config, typename, typename>
    MEM_STRONG_INLINE std::vector<pointer> memory_scanner::scan(Scanner&& scanner, Config&& config) const
    {
        if (!scanner.is_ready())
        {
            return {};
        }

        std::vector<pointer> results;

        size_t overlap = scanner.pattern_size() - 1;
        std::vector<byte> buffer(config.block_size + overlap);
        region scan_region(buffer.data(), buffer.size());

        void* current = config.start;

        while (current < config.end)
        {
            region_info region_info = {};
            if (!accessor_.query_region(current, region_info))
            {
                break;
            }

            if (region_info.flags == config.flags)
            {
                size_t base_address = reinterpret_cast<size_t>(region_info.start);
                size_t region_size = region_info.size;

                size_t scan_start = std::max(reinterpret_cast<size_t>(current), base_address);
                size_t scan_end = std::min(reinterpret_cast<size_t>(config.end), base_address + region_size);
                size_t scan_size = scan_end - scan_start;

                for (size_t read_pos = scan_start; read_pos < scan_end; read_pos += config.block_size)
                {
                    scan_region.size = std::min(config.block_size + overlap, scan_end - read_pos);
                    if (!accessor_.read(reinterpret_cast<void*>(read_pos), buffer.data(), scan_region.size))
                        continue;

                    scanner.scan_all(scan_region, [&](const pointer& p) {
                        results.push_back(p - buffer.data() + read_pos);
                        return false;
                    });
                }
            }

            current = static_cast<byte*>(region_info.start) + region_info.size;
        }

        return results;
    }

    template <typename Scanner, typename... Args, typename Config, typename, typename>
    MEM_STRONG_INLINE auto memory_scanner::scan(Config&& config, Args&&... args) const
    {
        Scanner scanner(std::forward<Args>(args)...);
        return scan<Scanner>(std::move(scanner), std::forward<Config>(config));
    }

    MEM_STRONG_INLINE memory_scanner& get_default_scanner()
    {
        static memory_scanner instance(current_process_accessor::get_instance());
        return instance;
    }

    template <typename Scanner, typename... Args>
    MEM_STRONG_INLINE constexpr auto memory_scanner::scan_default(Args&&... args)
    {
        return get_default_scanner().scan<Scanner>(std::forward<Args>(args)...);
    }

    MEM_STRONG_INLINE constexpr const data_accessor& memory_scanner::get_accessor() const
    {
        return accessor_;
    }
} // namespace mem

#endif // MEMORY_SCANNER_H