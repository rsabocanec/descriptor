#ifndef RSABOCANEC_DATAGRAM_SOCKET_H
#define RSABOCANEC_DATAGRAM_SOCKET_H

#include "socket.h"

namespace rsabocanec {

class datagram_socket : public socket {
public:
    [[nodiscard]] int32_t read_from() const noexcept {
        return -1;
    }

    [[nodiscard]] int32_t write_to() const noexcept {
        return -1;
    }
};

} // rsabocanec

#endif //RSABOCANEC_DATAGRAM_SOCKET_H
