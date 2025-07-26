#include "can_socket.h"

#include <cstring>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <unistd.h>

namespace rsabocanec {
int32_t can_socket::open() noexcept {
    auto result = close();
    if (result == 0) {
        descriptor_ = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (descriptor_ == -1) {
            result = errno;
        }
    }

    return result;
}

int32_t can_socket::bind(std::string_view address) noexcept {
    if (descriptor_ == -1) {
        if (auto const result = open(); result != 0) {
            return result;
        }
    }

    if (address.empty()) {
        return EINVAL;
    }

    struct ifreq ifr{};
    ::strcpy(ifr.ifr_name, address.data());

    if (::ioctl(descriptor_, SIOCGIFINDEX, &ifr) == -1) {
        return errno;
    }

    const struct sockaddr_can can_address{
        .can_family = AF_CAN,
        .can_ifindex = ifr.ifr_ifindex,
    };

    if (::bind(descriptor_,
        static_cast<const struct sockaddr *>(static_cast<const void*>(&can_address)),
        sizeof(can_address)) == -1) {
        return errno;
    }

    return 0;
}
} // rsabocanec