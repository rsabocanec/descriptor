#include "stream_socket.h"

#include <array>

#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace descriptor {
std::error_code stream_socket::listen(int32_t max_connections) const noexcept {
    if (descriptor_.load() == -1) {
        return std::make_error_code(std::errc::bad_file_descriptor);
    }

    if (max_connections == -1) {
        max_connections = SOMAXCONN;
    }

    if (::listen(descriptor_.load(), max_connections) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    return {};
}

std::error_code stream_socket::disconnect() noexcept {
    if (descriptor_.load() == -1) {
        return std::make_error_code(std::errc::bad_file_descriptor);
    }

    if (::shutdown(descriptor_.load(), SHUT_RD) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    if (::shutdown(descriptor_.load(), SHUT_WR) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }
    else {
        char c{};
        if (auto const result = ::recv(descriptor_.load(), &c, 1, 0); result == -1) {
            return std::make_error_code(static_cast<std::errc>(errno));
        }
    }

    return close();
}
} // descriptor