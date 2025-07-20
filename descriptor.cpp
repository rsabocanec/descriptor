#include "descriptor.h"

#include <cstring>

#include <sys/types.h>
#include <unistd.h>

namespace rsabocanec {

int32_t descriptor::close() noexcept {
    if (descriptor_ != -1) {
        if (::close(descriptor_) == -1) {
            return errno;
        }

        descriptor_ = -1;
    }

    return 0;
}

std::tuple<int32_t, int32_t> descriptor::read(std::span<std::byte> buffer) const noexcept {
    std::tuple<int32_t, std::size_t> result {EINVAL, -1};

    if (descriptor_ != -1) {
        std::get<1>(result) =
            ::read(descriptor_,
                    static_cast<void*>(buffer.data()),
                    static_cast<std::size_t>(buffer.size()));

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

std::tuple<int32_t, int32_t> descriptor::write(std::span<const std::byte> buffer) const noexcept {
    std::tuple<int32_t, std::size_t> result {EINVAL, -1};

    if (descriptor_ != -1) {
        std::get<1>(result) =
            ::write(descriptor_,
                    static_cast<const void*>(buffer.data()),
                    static_cast<std::size_t>(buffer.size()));

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

int32_t descriptor::select(int32_t timeout) const noexcept {
    return -1;
}

int32_t descriptor::poll(int32_t timeout) const noexcept {
    return -1;
}

std::string_view descriptor::error_description(int32_t error_code) noexcept {
    return ::strerror(error_code);
}
}
