#include "socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace rsabocanec {

std::tuple<int32_t, int32_t> socket::splice_impl(const descriptor& source, std::size_t count,
                                                 std::optional<int32_t> offset/* = std::nullopt*/) const noexcept {

    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    off_t offset_value = offset ? static_cast<off_t>(*offset) : 0;

    if (descriptor_.load() != -1) {
        std::get<1>(result) =
            ::sendfile(descriptor_.load(),
                       source.get(),  
                       offset ? &offset_value : nullptr, 
                       count);

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

int32_t socket::shutdown() const noexcept {
    if (descriptor_.load() != -1) {
        if (::shutdown(descriptor_.load(), SHUT_RD) == -1) {
            return errno;
        }
    }

    return 0;
}

} // rsabocanec
