#include "local_stream_socket.h"

#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace rsabocanec {
int32_t local_stream_socket::open() noexcept {
    auto result = close();
    if (result == 0) {
        descriptor_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (descriptor_ == -1) {
            result = errno;
        }
    }

    return result;
}

int32_t local_stream_socket::bind(std::string_view address) noexcept {
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
                    sizeof(local_address)) == -1) {
        return errno;
                    }

    return 0;
}

std::expected<acceptor, int32_t> local_stream_socket::accept() const noexcept {
    if (descriptor_ == -1) {
        return std::unexpected(-1);
    }

    struct sockaddr_un client_address{
            .sun_family = AF_UNIX
    };

    socklen_t client_address_length{sizeof(client_address)};

    auto const result =
        ::accept(descriptor_,
            static_cast<sockaddr *>(static_cast<void *>(&client_address)), &client_address_length);

    if (result == -1) {
        return std::unexpected(errno);
    }

    return acceptor{result};
}

int32_t local_stream_socket::connect(std::string_view address) noexcept {
    if (address.empty()) {
        return EINVAL;
    }

    if (descriptor_ == -1) {
        if (auto const result = open(); result != 0) {
            return result;
        }
    }

    if (address.empty()) {
        return EINVAL;
    }

    struct sockaddr_un local_address{};
    local_address.sun_family = AF_UNIX;
    ::strncpy(local_address.sun_path, address.data(), address.length());

    if (::connect(descriptor_,
                    static_cast<const struct sockaddr *>(static_cast<const void*>(&local_address)),
                    sizeof(local_address)) == -1) {
        return errno;
    }

    return 0;
}
} // rsabocanec