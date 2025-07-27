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
std::tuple<int32_t, int32_t> can_frame::as_bytes(std::span<std::byte> buffer) const noexcept {
    std::tuple<int32_t, int32_t> result{EINVAL, -1};

    if (buffer.size() >= can_frame_buffer_size) {
        auto& count = std::get<1>(result);

        count = 0;

        const auto *ptr = static_cast<const std::byte*>(static_cast<const void*>(&header_.uint32_id_));
        std::copy_n(ptr, sizeof(can_header), std::begin(buffer));
        count += sizeof(can_header);

        count += sizeof(frame_type_);
        buffer[count] = static_cast<std::byte>(static_cast<uint8_t>(frame_type_));

        count += sizeof(payload_length_);
        buffer[count] = static_cast<std::byte>(payload_length_);

        auto const payload_as_bytes = std::as_bytes(std::span(payload_));
        std::copy_n(  payload_as_bytes.cbegin(), payload_length_,
                    buffer.begin() + count);
    }

    return result;
}

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