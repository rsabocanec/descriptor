#include "descriptor.h"

#include <cstring>

#include <sys/types.h>

#define _FILE_OFFSET_BITS 64
#include <unistd.h>

namespace rsabocanec {

int32_t descriptor::close() noexcept {
    if (descriptor_.load() != -1) {
        if (::close(descriptor_.load()) == -1) {
            return errno;
        }

        descriptor_ = -1;
    }

    return 0;
}

std::tuple<int32_t, int32_t> descriptor::read(std::span<std::byte> buffer) const noexcept {
    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    if (descriptor_.load() != -1) {
        std::get<1>(result) =
            ::read(descriptor_.load(),
                    static_cast<void*>(buffer.data()),
                    static_cast<std::size_t>(buffer.size()));

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

std::tuple<int32_t, int32_t> descriptor::write(std::span<const std::byte> buffer) const noexcept {
    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    if (descriptor_.load() != -1) {
        std::get<1>(result) =
            ::write(descriptor_.load(),
                    static_cast<const void*>(buffer.data()),
                    static_cast<std::size_t>(buffer.size()));

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

std::tuple<int32_t, int32_t> descriptor::splice(const descriptor& source, std::size_t count,
                                                std::optional<int32_t> offset_in/* = std::nullopt*/,
                                                std::optional<int32_t> offset_out/* = std::nullopt*/,
                                                uint32_t flags/* = 0ul*/) const noexcept {

    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    off_t offset_in_value = offset_in ? static_cast<off_t>(*offset_in) : 0;
    off_t offset_out_value = offset_out ? static_cast<off_t>(*offset_out) : 0;

    if (descriptor_.load() != -1) {
        std::get<1>(result) =
            ::copy_file_range(source.descriptor_.load(), 
                              offset_in ? &offset_in_value : nullptr, 
                              descriptor_.load(), 
                              offset_out ? &offset_out_value : nullptr, 
                              count, flags);

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

int32_t descriptor::select(int32_t/* timeout*/) const noexcept {
    return -1;
}

int32_t descriptor::poll(int32_t/* timeout*/) const noexcept {
    return -1;
}

std::string_view descriptor::error_description(int32_t error_code) noexcept {
    return ::strerror(error_code);
}
}
