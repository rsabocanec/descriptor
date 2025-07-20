/*
Copyright (c) 2025 robert.sabocanec@gmail.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef RSABOCANEC_STREAM_SOCKET_H
#define RSABOCANEC_STREAM_SOCKET_H

#pragma once

#include "bound_socket.h"

#include <string>
#include <expected>

namespace rsabocanec {

class acceptor : public socket {
    std::string peer_address_{};
    uint16_t peer_port_{};

public:
    acceptor() = delete;

    explicit acceptor(int32_t descriptor) noexcept
    : socket(descriptor){
    }

    explicit acceptor(int32_t descriptor, std::string&& peer_address, uint16_t peer_port) noexcept
    : socket(descriptor)
    , peer_address_(std::forward<std::string>(peer_address))
    , peer_port_(peer_port) {
    }

    explicit acceptor(int32_t descriptor, std::string&& peer_address) noexcept
    : socket(descriptor)
    , peer_address_(std::forward<std::string>(peer_address)) {
    }

    acceptor(const acceptor&) = delete;
    acceptor(acceptor&&) = default;

    acceptor& operator=(const acceptor&) = delete;
    acceptor& operator=(acceptor&&) = default;

    ~acceptor() noexcept override {
        [[maybe_unused]] auto const result = socket::shutdown();
    }

    [[nodiscard]] int32_t open() noexcept final {
        return 0;
    }

    [[nodiscard]] std::string_view peer() const noexcept {
        return peer_address_;
    }

    [[nodiscard]] uint16_t peer_port() const noexcept {
        return peer_port_;
    }
};

class stream_socket : public bound_socket {
public:
    stream_socket() = default;

    [[nodiscard]] int32_t listen(int32_t max_connections = -1) const noexcept;

    [[nodiscard]] virtual std::expected<acceptor, int32_t> accept() const noexcept = 0;

    [[nodiscard]] virtual int32_t connect(std::string_view address) noexcept = 0;

    [[nodiscard]] int32_t disconnect() noexcept {
        return close();
    }
};

} // rsabocanec

#endif //RSABOCANEC_STREAM_SOCKET_H
