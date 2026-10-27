#include "local_stream_socket.h"

#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

namespace descriptor {
std::error_code local_stream_socket::create() noexcept {
    auto result = close();
    if (!result) {
        descriptor_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (descriptor_.load() == -1) {
            result = std::make_error_code(static_cast<std::errc>(errno));
        }
    }

    return result;
}

std::error_code local_stream_socket::bind(std::string_view address) noexcept {
    if (address.empty()) {
        return std::make_error_code(std::errc::invalid_argument);
    }

    if (descriptor_.load() == -1) {
        if (auto const result = create()) {
            return result;
        }
    }

    struct sockaddr_un local_address{};
    local_address.sun_family = AF_UNIX;
    ::strncpy(local_address.sun_path, address.data(), address.length());

    if (::bind(descriptor_.load(),
                    static_cast<const struct sockaddr *>(static_cast<const void*>(&local_address)),
                    sizeof(local_address)) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
                    }

    return {};
}

std::expected<acceptor, std::error_code> local_stream_socket::accept() const noexcept {
    if (descriptor_.load() == -1) {
        return std::unexpected(std::make_error_code(std::errc::bad_file_descriptor));
    }

    struct sockaddr_un client_address{
            .sun_family = AF_UNIX
    };

    socklen_t client_address_length{sizeof(client_address)};

    auto const result =
        ::accept(descriptor_.load(),
            static_cast<sockaddr *>(static_cast<void *>(&client_address)), &client_address_length);

    if (result == -1) {
        return std::unexpected(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return acceptor{result};
}

std::error_code local_stream_socket::connect(std::string_view address) noexcept {
    if (address.empty()) {
        return std::make_error_code(std::errc::invalid_argument);
    }

    if (descriptor_.load() == -1) {
        if (auto const result = create()) {
            return result;
        }
    }

    struct sockaddr_un local_address{};
    local_address.sun_family = AF_UNIX;
    ::strncpy(local_address.sun_path, address.data(), address.length());

    if (::connect(descriptor_.load(),
                    static_cast<const struct sockaddr *>(static_cast<const void*>(&local_address)),
                    sizeof(local_address)) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    return {};
}
} // descriptor