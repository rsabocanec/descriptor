/*
Copyright (c) 2025 robert.sabocanec@gmail.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef RSABOCANEC_LOCAL_DATAGRAM_SOCKET_H
#define RSABOCANEC_LOCAL_DATAGRAM_SOCKET_H

#pragma once

#include "datagram_socket.h"

namespace rsabocanec {

class local_datagram_socket : public datagram_socket {
public:
    local_datagram_socket() = default;

    [[nodiscard]] int32_t open() noexcept final;
    [[nodiscard]] int32_t bind(std::string_view address) noexcept final;

    [[nodiscard]] std::tuple<int32_t, int32_t>
        read_from(std::span<std::byte> buffer, std::string &address) const noexcept final;

    [[nodiscard]] std::tuple<int32_t, int32_t>
        write_to(std::span<const std::byte> buffer, std::string_view address) const noexcept final;
};

} // rsabocanec

#endif //RSABOCANEC_LOCAL_DATAGRAM_SOCKET_H
