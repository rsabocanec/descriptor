#ifndef RSABOCANEC_TCP_SOCKET_H
#define RSABOCANEC_TCP_SOCKET_H

#pragma once

#include "stream_socket.h"

namespace rsabocanec {

class tcp_socket : public stream_socket {
public:
    tcp_socket() = default;

    [[nodiscard]] int32_t open() noexcept override;
};

} // rsabocanec

#endif //RSABOCANEC_TCP_SOCKET_H
