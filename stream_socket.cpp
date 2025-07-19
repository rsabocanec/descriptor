#include "stream_socket.h"

#include <array>

#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace rsabocanec {

int32_t stream_socket::bind(std::string_view address, uint16_t port) noexcept {
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
        .sin_addr = {.s_addr = ::htonl(INADDR_ANY)}};

    if (!address.empty()) {
        if (::inet_pton(AF_INET, address.data(), &server_address.sin_addr) == -1) {
            return errno;
        }
    }

    if (::bind(descriptor_,
                    static_cast<const struct sockaddr *>(static_cast<const void*>(&server_address)),
                    sizeof(server_address)) == -1) {
        return errno;
                    }

    return 0;
}

int32_t stream_socket::listen(int32_t max_connections) const noexcept {
    if (descriptor_ == -1) {
        return -1;
    }

    if (::listen(descriptor_, max_connections) == -1) {
        return errno;
    }

    return 0;
}

std::expected<acceptor, int32_t> stream_socket::accept() const noexcept {
    if (descriptor_ == -1) {
        return std::unexpected(-1);
    }

    struct sockaddr_in client_address{};
    socklen_t client_length{sizeof(client_address)};

    auto const result =
        ::accept(descriptor_,
            static_cast<sockaddr *>(static_cast<void *>(&client_address)), &client_length);

    if (result == -1) {
        return std::unexpected(errno);
    }

    std::array<char, INET_ADDRSTRLEN> buffer{};
    ::inet_ntop(AF_INET, static_cast<const void *>(&client_address.sin_addr),
                buffer.data(), buffer.max_size());

    std::string peer_address{buffer.data()};

    return acceptor{result, std::move(peer_address), ::ntohs(client_address.sin_port)};
}

int32_t stream_socket::connect(std::string_view address, uint16_t port) noexcept {
    if (descriptor_ == -1) {
        if (auto const result = open(); result != 0) {
            return result;
        }
    }

    if (address.empty()) {
        return EINVAL;
    }

    if (port == 0) {
        return EINVAL;
    }

    struct sockaddr_in server_address{.sin_family = AF_INET, .sin_port = ::htons(port)};

    if (::inet_pton(AF_INET, address.data(), &server_address.sin_addr) == -1) {
        return errno;
    }

    if (::connect(descriptor_,
                    static_cast<const struct sockaddr *>(static_cast<const void*>(&server_address)),
                    sizeof(server_address)) == -1) {
        return errno;
    }

    return 0;
}
} // rsabocanec