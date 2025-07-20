/*
Copyright (c) 2025 robert.sabocanec@gmail.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef RSABOCANEC_TCP_SOCKET_H
#define RSABOCANEC_TCP_SOCKET_H

#pragma once

#include "stream_socket.h"

namespace rsabocanec {

class tcp_socket : public stream_socket {
public:
    tcp_socket() = default;

    [[nodiscard]] int32_t open() noexcept final;

    [[nodiscard]] int32_t bind(std::string_view address) noexcept final {
        auto const [result, pure_address, port] = parse_address(address);

        if (result != 0) {
            return result;
        }

        return bind(pure_address, port);
    }

    [[nodiscard]] int32_t bind(std::string_view address, uint16_t port) noexcept;

    [[nodiscard]] std::expected<acceptor, int32_t> accept() const noexcept final;

    [[nodiscard]] int32_t connect(std::string_view address) noexcept final {
        auto const [result, pure_address, port] = parse_address(address);

        if (result != 0) {
            return result;
        }

        return connect(pure_address, port);
    }

    [[nodiscard]] int32_t connect(std::string_view address, uint16_t port) noexcept;
};

} // rsabocanec

#endif //RSABOCANEC_TCP_SOCKET_H
