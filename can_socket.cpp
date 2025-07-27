#include "can_socket.h"

#include <cstring>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>

#include <unistd.h>
#include <netinet/in.h>

namespace rsabocanec {
can_frame::can_frame(std::span<const std::byte> buffer) noexcept
: can_frame() {
    if (buffer.size() >= can_frame_buffer_size) {
        uint32_t id_value{};
        std::copy_n(buffer.cbegin(), sizeof(uint32_t),
                    static_cast<std::byte*>(static_cast<void*>(&id_value)));

        header(::ntohl(id_value));

        frame_type_ = static_cast<can_type>(static_cast<uint8_t>(buffer[sizeof(uint32_t)]));
        payload_length_ = static_cast<uint8_t>(buffer[sizeof(uint32_t) + 1]);

        std::copy_n(buffer.cbegin() + sizeof(uint32_t) + 2,
                    payload_length_,
                    std::as_writable_bytes(std::span(payload_)).begin());
    }
}

std::tuple<int32_t, int32_t> can_frame::as_bytes(std::span<std::byte> buffer) const noexcept {
    std::tuple<int32_t, int32_t> result{EINVAL, -1};

    if (buffer.size() >= can_frame_buffer_size) {
        auto& count = std::get<1>(result);

        count = 0;

        auto const id_value = ::htonl(header_.uint32_id_);
        const auto *ptr = static_cast<const std::byte*>(static_cast<const void*>(&id_value));
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