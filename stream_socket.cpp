#include "stream_socket.h"

#include <array>

#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace rsabocanec {
int32_t stream_socket::listen(int32_t max_connections) const noexcept {
    if (descriptor_ == -1) {
        return -1;
    }

    if (max_connections == -1) {
        max_connections = SOMAXCONN;
    }

    if (::listen(descriptor_, max_connections) == -1) {
        return errno;
    }

    return 0;
}

int32_t stream_socket::disconnect() noexcept {
    if (descriptor_ == -1) {
        return -1;
    }

    if (::shutdown(descriptor_, SHUT_RD) == -1) {
        return errno;
    }

    if (::shutdown(descriptor_, SHUT_WR) == -1) {
        return errno;
    }
    else {
        char c{};
        if (auto const result = ::recv(descriptor_, &c, 1, 0); result == -1) {
            return errno;
        }
    }

    return close();
}
} // rsabocanec