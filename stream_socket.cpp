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
} // rsabocanec