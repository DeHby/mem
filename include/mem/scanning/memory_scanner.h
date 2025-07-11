#ifndef MEMORY_SCANNER_H
#define MEMORY_SCANNER_H

#include <mem/access/remote_memory_accessor.h>
#include <mem/scanning/pattern.h>

#include "scan_config.h"
#include "simd_scanner.h"

#include <functional>
#include <vector>

namespace mem
{
    class memory_scanner
    {
    private:
        data_accessor& accessor_;

    public:
        memory_scanner(data_accessor& accessor);

        template <typename Scanner = boyer_moore_scanner, typename = is_scanner<Scanner>>
        std::vector<pointer> scan(Scanner& scanner, const scan_config& config) const;

        template <typename Scanner = boyer_moore_scanner, typename... Args, typename = is_scanner<Scanner>>
        auto scan(const scan_config& config, Args... args) const;

        template <typename Scanner = boyer_moore_scanner, typename... Args>
        static auto scan_default(Args&&... args);
    };

    memory_scanner::memory_scanner(data_accessor& accessor)
        : accessor_(accessor)
    {}

    template <typename Scanner, typename>
    std::vector<pointer> memory_scanner::scan(Scanner& scanner, const scan_config& config) const
    {
        if (!scanner.is_ready())
        {
            return {};
        }

        std::vector<pointer> results;
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

                size_t overlap = scanner.pattern_size() - 1;
                std::vector<byte> buffer(config.block_size + overlap);
                region scan_region(buffer.data(), buffer.size());

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

    template <typename Scanner, typename... Args, typename>
    auto memory_scanner::scan(const scan_config& config, Args... args) const
    {
        Scanner scanner(std::forward<Args>(args)...);
        return scan<Scanner>(scanner, config);
    }

    MEM_STRONG_INLINE memory_scanner& get_default_scanner()
    {
        static memory_scanner instance(get_default_accessor());
        return instance;
    }

    template <typename Scanner, typename... Args>
    auto memory_scanner::scan_default(Args&&... args)
    {
        return get_default_scanner().scan<Scanner>(std::forward<Args>(args)...);
    }
} // namespace mem

#endif // MEMORY_SCANNER_H