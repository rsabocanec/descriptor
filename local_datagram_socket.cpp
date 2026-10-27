#include "local_datagram_socket.h"

#include <sys/socket.h>
#include <sys/un.h>

namespace descriptor {

std::error_code local_datagram_socket::create() noexcept {
    auto result = close();
    if (!result) {
        descriptor_ = ::socket(AF_UNIX, SOCK_DGRAM, 0);
        if (descriptor_.load() == -1) {
            result = std::make_error_code(static_cast<std::errc>(errno));
        }
    }

    return result;
}

std::error_code local_datagram_socket::bind(std::string_view address) noexcept {
    if (address.empty()) {
        return std::make_error_code(std::errc::bad_file_descriptor);
    }

    if (descriptor_.load() == -1) {
        if (auto const result = create()) {
            return result;
        }
    }

    struct sockaddr_un local_address{};
    local_address.sun_family = AF_UNIX;
    ::strncpy(local_address.sun_path, address.data(), address.length());

    if (::bind(descriptor_.load(),
                    static_cast<const struct sockaddr *>(static_cast<const void*>(&local_address)),
                    sizeof(local_address.sun_family) + address.length()) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
                    }

    return {};
}

expected_result local_datagram_socket::read_from(
    std::span<std::byte> buffer, std::string &address) const noexcept {

    if (descriptor_.load() != -1) {
        struct sockaddr_un local_address{
            .sun_family = AF_UNIX
        };

        socklen_t local_address_length{sizeof(local_address)};

        auto const result =
            ::recvfrom(descriptor_.load(),
                    static_cast<void*>(buffer.data()),
                    static_cast<std::size_t>(buffer.size()),
                    0,
                    static_cast<struct sockaddr *>(static_cast<void *>(&local_address)),
                    &local_address_length);

        if (result == -1) {
            return std::unexpected(std::make_error_code(static_cast<std::errc>(errno)));
        }
        else {
            address = std::string(&local_address.sun_path[0],
                                  local_address_length - sizeof(local_address.sun_family));
            return expected_result{static_cast<int32_t>(result)};
        }
    }

    return std::unexpected(std::make_error_code(std::errc::bad_file_descriptor));
}

expected_result local_datagram_socket::write_to(
    std::span<const std::byte> buffer, std::string_view address) const noexcept {

    if (descriptor_.load() != -1) {
        struct sockaddr_un local_address{
            .sun_family = AF_UNIX
        };

        ::strncpy(local_address.sun_path, address.data(), address.length());

        auto const result =
            ::sendto(descriptor_.load(),
                    static_cast<const void*>(buffer.data()),
                    static_cast<std::size_t>(buffer.size()),
                    0,
                    static_cast<const struct sockaddr *>(static_cast<const void *>(&local_address)),
                    sizeof(local_address.sun_family) + address.length());

        return result != -1 ? expected_result{static_cast<int32_t>(result)} 
                            : std::unexpected(std::make_error_code(static_cast<std::errc>(errno)));    
    }

    return std::unexpected(std::make_error_code(std::errc::bad_file_descriptor));
}
} // descriptor