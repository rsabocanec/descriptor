#include "socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace descriptor {

expected_result socket::splice_impl(const descriptor& source, std::size_t count,
                                                 std::optional<int32_t> offset/* = std::nullopt*/) const noexcept {

    off_t offset_value = offset ? static_cast<off_t>(*offset) : 0;

    if (descriptor_.load() != -1) {
        auto const result =
            ::sendfile(descriptor_.load(),
                       source.load(),  
                       offset ? &offset_value : nullptr, 
                       count);

        return result != -1 ? expected_result{result} 
                            : std::unexpected(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
}

int32_t socket::shutdown() const noexcept {
    if (descriptor_.load() != -1) {
        if (::shutdown(descriptor_.load(), SHUT_RD) == -1) {
            return errno;
        }
    }

    return 0;
}

} // descriptor
