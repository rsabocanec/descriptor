/*
Copyright (c) 2025 robert.sabocanec@gmail.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef RSABOCANEC_DATAGRAM_SOCKET_H
#define RSABOCANEC_DATAGRAM_SOCKET_H

#include "bound_socket.h"

#include <tuple>

namespace rsabocanec {

class datagram_socket : public bound_socket {
public:
    datagram_socket() = default;

    template<std::random_access_iterator It>
    [[nodiscard]] std::tuple<int32_t, int32_t> read_from(It first, std::size_t count, std::string &address) const noexcept {
        return read_from(std::as_writable_bytes(std::span(first, count)), address);
    }

    template<std::random_access_iterator It, class End>
    [[nodiscard]] std::tuple<int32_t, int32_t> read_from(It first, End last, std::string &address) const noexcept {
        return read_from(std::as_writable_bytes(std::span(first, last)), address);
    }

    template<class U, std::size_t N>
    [[nodiscard]] std::tuple<int32_t, int32_t> read_from(std::array<U, N>& arr, std::string &address) const noexcept {
        return read_from(std::as_writable_bytes(std::span(arr)), address);
    }

    [[nodiscard]] virtual std::tuple<int32_t, int32_t>
        read_from(std::span<std::byte> buffer, std::string &address) const noexcept = 0;

    template<std::random_access_iterator It>
    [[nodiscard]] std::tuple<int32_t, int32_t> write_to(It first, std::size_t count, std::string_view address) const noexcept {
        return write_to(std::as_bytes(std::span(first, count)), address);
    }

    template<std::random_access_iterator It, class End>
    [[nodiscard]] std::tuple<int32_t, int32_t> write_to(It first, End last, std::string_view address) const noexcept {
        return write_to(std::as_bytes(std::span(first, last)), address);
    }

    template<class U, std::size_t N>
    [[nodiscard]] std::tuple<int32_t, int32_t> write_to(const std::array<U, N>& arr, std::string_view address) const noexcept {
        return write_to(std::as_bytes(std::span(arr)), address);
    }

    [[nodiscard]] virtual std::tuple<int32_t, int32_t>
        write_to(std::span<const std::byte> buffer, std::string_view address) const noexcept = 0;
};

} // rsabocanec

#endif //RSABOCANEC_DATAGRAM_SOCKET_H
