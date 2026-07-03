#include "socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace rsabocanec {

std::tuple<int32_t, int32_t> socket::splice(std::span<const std::byte> buffer) const noexcept {
    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    return result;
}

int32_t socket::shutdown() const noexcept {
    if (descriptor_ != -1) {
        if (::shutdown(descriptor_, SHUT_RD) == -1) {
            return errno;
        }
    }

    return 0;
}

} // rsabocanec
