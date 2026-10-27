#include "tcp_socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace descriptor {

std::error_code tcp_socket::create() noexcept {
    auto result = close();
    if (!result) {
        descriptor_ = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (descriptor_.load() == -1) {
            result = std::make_error_code(static_cast<std::errc>(errno));
        }
    }

    return result;
}

std::error_code tcp_socket::bind(std::string_view address, uint16_t port) noexcept {
    if (descriptor_.load() == -1) {
        if (auto const result = create()) {
            return result;
        }
    }

    if (port == 0) {
        return std::make_error_code(std::errc::invalid_argument);
    }

    struct sockaddr_in server_address{
        .sin_family = AF_INET,
        .sin_port = ::htons(port),
        .sin_addr = {.s_addr = ::htonl(INADDR_ANY)}};

    if (!address.empty()) {
        if (::inet_pton(AF_INET, address.data(), &server_address.sin_addr) == -1) {
            return std::make_error_code(static_cast<std::errc>(errno));
        }
    }

    if (::bind(descriptor_.load(),
                    static_cast<const struct sockaddr *>(static_cast<const void*>(&server_address)),
                    sizeof(server_address)) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    return std::error_code{};
}

std::expected<acceptor, std::error_code> tcp_socket::accept() const noexcept {
    if (descriptor_.load() == -1) {
        return std::unexpected(std::make_error_code(std::errc::bad_file_descriptor));
    }

    struct sockaddr_in client_address{};
    socklen_t client_address_length{sizeof(client_address)};

    auto const result =
        ::accept(descriptor_.load(),
            static_cast<sockaddr *>(static_cast<void *>(&client_address)), &client_address_length);

    if (result == -1) {
        return std::unexpected(std::make_error_code(static_cast<std::errc>(errno)));
    }

    std::array<char, INET_ADDRSTRLEN> buffer{};
    ::inet_ntop(AF_INET, static_cast<const void *>(&client_address.sin_addr),
                buffer.data(), buffer.max_size());

    std::string peer_address{buffer.data()};

    return acceptor{result, std::move(peer_address), ::ntohs(client_address.sin_port)};
}

std::error_code tcp_socket::connect(std::string_view address, uint16_t port) noexcept {
    if (descriptor_.load() == -1) {
        if (auto const result = create()) {
            return result;
        }
    }

    if (address.empty()) {
        return std::make_error_code(std::errc::invalid_argument);
    }

    if (port == 0) {
        return std::make_error_code(std::errc::invalid_argument);
    }

    struct sockaddr_in server_address{.sin_family = AF_INET, .sin_port = ::htons(port)};

    if (::inet_pton(AF_INET, address.data(), &server_address.sin_addr) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    if (::connect(descriptor_.load(),
                    static_cast<const struct sockaddr *>(static_cast<const void*>(&server_address)),
                    sizeof(server_address)) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    return {};
}
} // descriptor