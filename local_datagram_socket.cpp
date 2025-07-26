#include "local_datagram_socket.h"

#include <sys/socket.h>
#include <sys/un.h>

namespace rsabocanec {

    int32_t local_datagram_socket::open() noexcept {
        auto result = close();
        if (result == 0) {
            descriptor_ = ::socket(AF_UNIX, SOCK_DGRAM, 0);
            if (descriptor_ == -1) {
                result = errno;
            }
        }

        return result;
}

    int32_t local_datagram_socket::bind(std::string_view address) noexcept {
        if (address.empty()) {
            return EINVAL;
        }

        if (descriptor_ == -1) {
            if (auto const result = open(); result != 0) {
                return result;
            }
        }

        struct sockaddr_un local_address{};
        local_address.sun_family = AF_UNIX;
        ::strncpy(local_address.sun_path, address.data(), address.length());

        if (::bind(descriptor_,
                        static_cast<const struct sockaddr *>(static_cast<const void*>(&local_address)),
                        sizeof(local_address.sun_family) + address.length()) == -1) {
            return errno;
                        }

        return 0;
}

std::tuple<int32_t, int32_t>
    local_datagram_socket::read_from(std::span<std::byte> buffer, std::string &address) const noexcept {
    std::tuple<int32_t, std::size_t> result {EINVAL, -1};

    if (descriptor_ != -1) {
        struct sockaddr_un local_address{
            .sun_family = AF_UNIX
        };

        socklen_t local_address_length{sizeof(local_address)};

        std::get<1>(result) =
            ::recvfrom(descriptor_,
                    static_cast<void*>(buffer.data()),
                    static_cast<std::size_t>(buffer.size()),
                    0,
                    static_cast<struct sockaddr *>(static_cast<void *>(&local_address)),
                    &local_address_length);

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

std::tuple<int32_t, int32_t>
    local_datagram_socket::write_to(std::span<const std::byte> buffer, std::string_view address) const noexcept {
    std::tuple<int32_t, std::size_t> result {EINVAL, -1};

    if (descriptor_ != -1) {
        struct sockaddr_un local_address{
            .sun_family = AF_UNIX
        };

        ::strncpy(local_address.sun_path, address.data(), address.length());

        std::get<1>(result) =
            ::sendto(descriptor_,
                    static_cast<const void*>(buffer.data()),
                    static_cast<std::size_t>(buffer.size()),
                    0,
                    static_cast<const struct sockaddr *>(static_cast<const void *>(&local_address)),
                    sizeof(local_address.sun_family) + address.length());

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}
} // rsabocanec