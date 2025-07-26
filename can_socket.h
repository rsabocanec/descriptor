/*
Copyright (c) 2025 robert.sabocanec@gmail.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef RSABOCANEC_CAN_SOCKET_H
#define RSABOCANEC_CAN_SOCKET_H

#pragma once

#include "bound_socket.h"

namespace rsabocanec {
struct can_header {
    static constexpr std::size_t msg_num_bits   = 6ull;
    static constexpr std::size_t id_bits        = 8ull;
    static constexpr std::size_t dest_bits      = 7ull;
    static constexpr std::size_t src_bits       = 7ull;
    static constexpr std::size_t type_bits      = 1ull;
    static constexpr std::size_t reserved_bits  = 3ull;

    union {
        struct {
            uint32_t msg_num_   : msg_num_bits;
            uint32_t msg_id_    : id_bits;
            uint32_t dest_      : dest_bits;
            uint32_t src_       : src_bits;
            uint32_t msg_type_  : type_bits;
            uint32_t            : reserved_bits;
        } bitfield_id_;
        uint32_t uint32_id_{};
    };

    can_header() = default;
    explicit can_header(uint32_t id) noexcept : uint32_id_(id) {}

    auto operator<=>(const can_header &other) const noexcept {
        return uint32_id_ <=> other.uint32_id_;
    }
};

enum class can_type : uint8_t {
    classical_frame = 0x00,
    remote_frame = 0x01,
    extended_frame = 0x02,
    fd_frame = 0x04,
    fd_bit_rate_switch = 0x08,
    fd_error_state = 0x10,
    error_frame = 0x40,
    status_frame = 0x80
};

    /*
    - 0x00U The PCAN message is a CAN Standard Frame (11-bit identifier)
    - 0x01U  The PCAN message is a CAN Remote-Transfer-Request Frame
    - 0x02U  The PCAN message is a CAN Extended Frame (29-bit identifier)
    - 0x04U  The PCAN message represents a FD frame in terms of CiA Specs
    - 0x08U  The PCAN message represents a FD bit rate switch (CAN data at a
    higher bit rate)
        - 0x10U  The PCAN message represents a FD error state indicator(CAN FD
        transmitter was error active)
        - 0x40U  The PCAN message represents an error frame
        - 0x80U  The PCAN message represents a PCAN status message
    */
using can_payload = std::array<uint8_t, sizeof(uint64_t)>;

class can_frame {
    can_header header_{};
    can_type frame_type_{can_type::extended_frame};
    can_payload payload_{};
    uint8_t payload_length_{0};

public:
    can_frame() = default;

    explicit can_frame(uint32_t header, can_type frame_type, can_payload payload, uint8_t payload_length) noexcept
    : header_(header), frame_type_(frame_type), payload_length_(payload_length), payload_(payload) {
    }

    can_frame(const can_frame&) = default;
    can_frame(can_frame&&) = default;
    can_frame& operator=(const can_frame&) = default;
    can_frame& operator=(can_frame&&) = default;
    ~can_frame() = default;

    auto operator<=>(const can_frame&) const = default;

    [[nodiscard]] auto header() const noexcept { return header_; }
    void header(uint32_t uint32_id) noexcept { header_.uint32_id_ = uint32_id; }
    void header(uint32_t msg_num, uint32_t msg_id, uint32_t dest, uint32_t src, uint32_t msg_type) noexcept {
        can_header header{};
        header.bitfield_id_ = {
            .msg_num_   = msg_num,
            .msg_id_    = msg_id,
            .dest_     = dest,
            .src_       = src,
            .msg_type_  = msg_type
        };

        header_.uint32_id_ = header.uint32_id_;
    }

    [[nodiscard]] auto fraame_type() const noexcept { return frame_type_; }
    void frame_type(can_type frame_type) noexcept { frame_type_ = frame_type; }

    [[nodiscard]] auto payload() const noexcept { return payload_; }
    void payload(const can_payload& payload) noexcept { payload_ = payload; }

    [[nodiscard]] auto payload_length() const noexcept { return payload_length_; }
    void payload_length(uint8_t payload_length) noexcept { payload_length_ = payload_length; }

    [[nodiscard]] auto msg_num() const noexcept { return header_.bitfield_id_.msg_num_; }
    [[nodiscard]] auto msg_id() const noexcept { return header_.bitfield_id_.msg_id_; }
    [[nodiscard]] auto dest() const noexcept { return header_.bitfield_id_.dest_; }
    [[nodiscard]] auto src() const noexcept { return header_.bitfield_id_.src_; }
    [[nodiscard]] auto msg_type() const noexcept { return header_.bitfield_id_.msg_type_; }
};

class can_socket : public bound_socket {
public:
    can_socket() = default;

    [[nodiscard]] int32_t open() noexcept final;
    [[nodiscard]] int32_t bind(std::string_view address) noexcept final;
};

} // rsabocanec

#endif //RSABOCANEC_CAN_SOCKET_H
