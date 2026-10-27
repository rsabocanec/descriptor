/*
Copyright (c) 2025 robert.sabocanec@gmail.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DESCRIPTOR_SOCKET_H
#define DESCRIPTOR_SOCKET_H

#pragma once

#include "descriptor.h"

#include <algorithm>
#include <charconv>

namespace descriptor {

using expected_parsing_result = std::expected<std::tuple<std::string_view, uint16_t>, std::error_code>;

[[nodiscard]] inline expected_parsing_result parse_ip_address(std::string_view address) noexcept {

    auto pos = std::ranges::find(address, ':');
    if (pos == address.cend()) {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    const std::string_view pure_address(address.cbegin(), pos);

    uint16_t port{};

    if (std::from_chars(++pos, address.cend(), port).ec == std::errc{}) {
        return expected_parsing_result({pure_address, port});
    }

    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
}

class socket : public descriptor {
public:
    socket() = default;
    explicit socket(int socket) : descriptor(socket) {}

    [[nodiscard]] expected_result splice(const descriptor& source, std::size_t count, 
                                         std::optional<int32_t> offset_in = std::nullopt,
                                         [[maybe_unused]] std::optional<int32_t> offset_out = std::nullopt,
                                         [[maybe_unused]] uint32_t flags = 0ul) const noexcept final override {
        return splice_impl(source, count, offset_in);
    }

    [[nodiscard]] virtual std::error_code shutdown() const noexcept;

protected:
    [[nodiscard]] expected_result splice_impl(const descriptor& source, std::size_t count, 
                                              std::optional<int32_t> offset = std::nullopt) const noexcept;
};

} // descriptor

#endif //DESCRIPTOR_SOCKET_H
