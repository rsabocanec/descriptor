#include "tcp_socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace rsabocanec {

int32_t tcp_socket::open() noexcept {
    auto result = close();
    if (result == 0) {
        descriptor_ = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (descriptor_ == -1) {
            result = errno;
        }
    }

    return result;
}

} // rsabocanec