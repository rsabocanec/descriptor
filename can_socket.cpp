#include "can_socket.h"

#include <cstring>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>

#include <netinet/in.h>

namespace descriptor {
std::size_t can_socket_frame::can_frame_buffer_size = sizeof(struct can_frame);

can_socket_frame::can_socket_frame(std::span<const std::byte> buffer) noexcept
: can_socket_frame() {
    if (buffer.size() >= can_frame_buffer_size) {
        const auto *ptr_frame = static_cast<const struct can_frame *>(static_cast<const void*>(buffer.data()));

        header(::ntohl(ptr_frame->can_id));

        if (ptr_frame->can_id & CAN_EFF_FLAG) frame_type_ = can_type::extended_frame;
        else if (ptr_frame->can_id & CAN_RTR_FLAG) frame_type_ = can_type::remote_frame;
        else if (ptr_frame->can_id & CAN_ERR_FLAG) frame_type_ = can_type::error_frame;
        else frame_type_ = can_type::standard_frame;

        payload_length_ = ptr_frame->len;

        std::copy_n(ptr_frame->data,
                    payload_length_,
                    payload_.begin());
    }
}

expected_result can_socket_frame::as_bytes(std::span<std::byte> buffer) const noexcept {

    struct can_frame frame{
        .can_id = header_.uint32_id_ & 0x0FFFFFFFU,
        .len = payload_length_
    };

    switch (frame_type_) {
      case can_type::standard_frame:
        frame.can_id &= CAN_SFF_MASK;
        break;
      case can_type::extended_frame:
        frame.can_id &= CAN_EFF_MASK;
        frame.can_id |= CAN_EFF_FLAG;
        break;
      case can_type::remote_frame:
        frame.can_id |= CAN_RTR_FLAG;
        break;
      case can_type::error_frame:
        frame.can_id |= CAN_ERR_FLAG;
        break;
      default:
        break;
    }

    ::memccpy(frame.data, payload_.data(), sizeof(frame.data), payload_length_);

    if (buffer.size() >= sizeof(frame)) {
        std::copy_n(static_cast<const uint8_t*>(static_cast<const void*>(&frame)),
                    sizeof(frame),
                    static_cast<uint8_t*>(static_cast<void*>(buffer.data())));
                    
        return expected_result{static_cast<int32_t>(sizeof(frame))};
    }

    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
}

std::error_code can_socket::create() noexcept {
    auto result = close();
    if (!result) {
        descriptor_ = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (descriptor_.load() == -1) {
            result = std::make_error_code(static_cast<std::errc>(errno));
        }
    }

    return result;
}

std::error_code can_socket::bind(std::string_view address) noexcept {
    if (descriptor_.load() == -1) {
        if (auto const result = create()) {
            return result;
        }
    }

    if (address.empty()) {
        return std::make_error_code(std::errc::invalid_argument);
    }

    struct ifreq ifr{};
    ::strcpy(ifr.ifr_name, address.data());

    if (::ioctl(descriptor_.load(), SIOCGIFINDEX, &ifr) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    const struct sockaddr_can can_address{
        .can_family = AF_CAN,
        .can_ifindex = ifr.ifr_ifindex,
    };

    if (::bind(descriptor_.load(),
        static_cast<const struct sockaddr *>(static_cast<const void*>(&can_address)),
        sizeof(can_address)) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    return {};
}
} // descriptor
