#include "udp_socket.h"

#include <string>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace rsabocanec {

int32_t udp_socket::open() noexcept {
    if (descriptor_ == -1) {
        if (auto const result = close(); result != 0) {
            return result;
        }
    }

    descriptor_ = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (descriptor_ == -1) {
        return errno;
    }

    return 0;
}

int32_t udp_socket::bind(std::string_view address, uint16_t port) noexcept {
    if (descriptor_ == -1) {
        if (auto const result = open(); result != 0) {
            return result;
        }
    }

    if (port == 0) {
        return EINVAL;
    }

    struct sockaddr_in server_address{
        .sin_family = AF_INET,
        .sin_port = ::htons(port),
        .sin_addr = {.s_addr = ::htonl(INADDR_ANY)}
    };

    if (!address.empty()) {
        if (::inet_pton(AF_INET, address.data(), &server_address.sin_addr) == -1) {
            return errno;
        }
    }

    if (::bind( descriptor_,
                static_cast<const struct sockaddr *>(static_cast<const void*>(&server_address)),
                sizeof(server_address)) == -1) {
        return errno;
    }

    return 0;
}

std::tuple<int32_t, int32_t> udp_socket::read_from(
        std::span<std::byte> buffer, std::string &address, uint16_t &port) const noexcept {
    std::tuple<int32_t, std::size_t> result {EINVAL, -1};

    if (descriptor_ != -1) {
        struct sockaddr_in peer_address{
            .sin_family = AF_INET,
            .sin_port = ::htons(port),
            .sin_addr = {.s_addr = ::htonl(INADDR_ANY)}
        };

        socklen_t peer_address_length = sizeof(peer_address);

        std::get<1>(result) =
            ::recvfrom( descriptor_,
                        static_cast<void*>(buffer.data()),
                        static_cast<std::size_t>(buffer.size()),
                        0,
                        static_cast<struct sockaddr *>(static_cast<void*>(&peer_address)),
                        &peer_address_length);

        if (std::get<1>(result) == -1) {
            std::get<0>(result) = errno;
        }
        else {
            std::array<char, INET_ADDRSTRLEN> input{};
            ::inet_ntop(AF_INET, static_cast<const void *>(&peer_address.sin_addr),
                        input.data(), input.max_size());

            address = input.data();
            port = ::ntohs(peer_address.sin_port);
        }
    }

    return result;
}

std::tuple<int32_t, int32_t> udp_socket::write_to(
        std::span<const std::byte> buffer, std::string_view address, uint16_t port) const noexcept {
    std::tuple<int32_t, std::size_t> result {EINVAL, -1};

    if (descriptor_ != -1) {
        const struct sockaddr_in peer_address{
            .sin_family = AF_INET,
            .sin_port = ::htons(port),
            .sin_addr = {.s_addr = ::inet_addr(address.data())}
        };

        std::get<1>(result) =
            ::sendto(descriptor_,
                    static_cast<const void*>(buffer.data()),
                    static_cast<std::size_t>(buffer.size()),
                    0,
                    static_cast<const struct sockaddr *>(static_cast<const void *>(&peer_address)),
                    sizeof(peer_address));

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}
} // rsabocanec