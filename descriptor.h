/*
Copyright (c) 2025 robert.sabocanec@gmail.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef RSABOCANEC_DESCRIPTOR_H
#define RSABOCANEC_DESCRIPTOR_H

#pragma once

#include <atomic>
#include <span>
#include <tuple>
#include <string_view>

#include <cstdint>

namespace rsabocanec {

class descriptor {
protected:
    std::atomic<int32_t> descriptor_{-1};

public:
    descriptor() = default;
    explicit descriptor(int descriptor) : descriptor_(descriptor) {}

    descriptor(const descriptor&) = delete;
    descriptor(descriptor&& other) noexcept {
        auto const tmp = other.descriptor_.load();
        other.descriptor_ = -1;
        descriptor_ = tmp;
    }

    descriptor& operator=(const descriptor&) = delete;

    descriptor& operator=(descriptor&& other) noexcept {
        if (this != &other) {
            auto tmp = other.descriptor_.load();
            other.descriptor_ = -1;
            descriptor_ = tmp;
        }

        return *this;
    }

    virtual ~descriptor() {
        [[maybe_unused]] auto const result = close();
    }

    [[nodiscard]] virtual int32_t open() noexcept = 0;
    [[nodiscard]] int32_t close() noexcept;

    [[nodiscard]] std::tuple<int32_t, int32_t> read(std::span<std::byte> buffer) const noexcept;
    [[nodiscard]] std::tuple<int32_t, int32_t> write(std::span<const std::byte> buffer) const noexcept;

    [[nodiscard]] int32_t select(int32_t timeout) const noexcept;
    [[nodiscard]] int32_t poll(int32_t timeout) const noexcept;

    [[nodiscard]] static std::string_view error_description(int32_t error_code) noexcept;
};

}
#endif //RSABOCANEC_DESCRIPTOR_H