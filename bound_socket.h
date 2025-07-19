#ifndef RSABOCANEC_BOUND_SOCKET_H
#define RSABOCANEC_BOUND_SOCKET_H

#pragma once

#include "socket.h"

#include <string_view>

namespace rsabocanec {

class bound_socket : public socket {
public:
    bound_socket() = default;

    [[nodiscard]] virtual int32_t bind(std::string_view address, uint16_t port = 0) noexcept = 0;
};

} // rsabocanec

#endif //RSABOCANEC_BOUND_SOCKET_H
