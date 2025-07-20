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
    , peer_address_(std::move(peer_address))
    , peer_port_(peer_port) {
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

    [[nodiscard]] int32_t bind(std::string_view address, uint16_t port = 0) noexcept override;
    [[nodiscard]] int32_t listen(int32_t max_connections = -1) const noexcept;

    [[nodiscard]] std::expected<acceptor, int32_t> accept() const noexcept;

    [[nodiscard]] int32_t connect(std::string_view address, uint16_t port) noexcept;
    [[nodiscard]] int32_t disconnect() noexcept {
        return close();
    }
};

} // rsabocanec

#endif //RSABOCANEC_STREAM_SOCKET_H
