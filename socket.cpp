#include "socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace rsabocanec {

int32_t socket::shutdown() const noexcept {
    if (descriptor_ != -1) {
        if (::shutdown(descriptor_, SHUT_RD) == -1) {
            return errno;
        }
    }

    return 0;
}

} // rsabocanec