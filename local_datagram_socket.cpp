#include "local_datagram_socket.h"

namespace rsabocanec {

int32_t local_datagram_socket::open() noexcept {
    return {};
}

int32_t local_datagram_socket::bind(std::string_view address) noexcept {
    return {};
}

std::tuple<int32_t, int32_t>
    local_datagram_socket::read_from(std::span<std::byte> buffer, std::string &address) const noexcept {
    return {};
}

std::tuple<int32_t, int32_t>
    local_datagram_socket::write_to(std::span<const std::byte> buffer, std::string_view address) const noexcept {
    return {};
}
} // rsabocanec