/*
Copyright (c) 2025 robert.sabocanec@gmail.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DESCRIPTOR_DESCRIPTOR_H
#define DESCRIPTOR_DESCRIPTOR_H

#pragma once

#include <chrono>
#include <atomic>
#include <span>
#include <tuple>
#include <optional>
#include <string_view>
#include <iterator>
#include <source_location>

#include <cstdint>

namespace std {
class os;
}

namespace descriptor {

template <typename Period>
using duration = std::chrono::duration<int64_t, Period>;

void report_error(
    std::ostream& os,
    int32_t error_num,
    std::string_view prefix = {},
    std::source_location location = std::source_location::current()) noexcept;

struct polled_state {
    bool has_something_to_read_{};
    bool has_something_to_write_{};
    bool has_error_{};
    bool has_hangup_{};
    bool has_peer_closed_{};
    bool has_invalid_request_{};
    bool has_exception_{};

    bool has_something_to_read() const noexcept {
        return has_something_to_read_;
    }

    bool has_something_to_write() const noexcept {
        return has_something_to_write_;
    }

    bool has_error() const noexcept {
        return has_error_;
    }

    bool has_hangup() const noexcept {
        return has_hangup_;
    }

    bool has_peer_closed() const noexcept {
        return has_peer_closed_;
    }

    bool has_invalid_request() const noexcept {
        return has_invalid_request_;
    }

    bool has_exception() const noexcept {
        return has_exception_;
    }

    void set_state(int16_t revents) noexcept;
};

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

    [[nodiscard]] bool valid() const noexcept {
        return descriptor_ >= 0;
    }
    
    [[nodiscard]] int32_t get() const noexcept {
        return descriptor_.load();
    }

protected:    
    [[nodiscard]] virtual int32_t create() noexcept = 0;

public:
    [[nodiscard]] virtual int32_t close() noexcept;

    template<std::random_access_iterator It>
    [[nodiscard]] std::tuple<int32_t, int32_t> read(It first, std::size_t count) const noexcept {
        return read(std::as_writable_bytes(std::span(first, count)));
    }

    template<std::random_access_iterator It, class End>
    [[nodiscard]] std::tuple<int32_t, int32_t> read(It first, End last) const noexcept {
        return read(std::as_writable_bytes(std::span(first, last)));
    }

    template<class U, std::size_t N>
    [[nodiscard]] std::tuple<int32_t, int32_t> read(std::array<U, N>& arr) const noexcept {
        return read(std::as_writable_bytes(std::span(arr)));
    }

    [[nodiscard]] std::tuple<int32_t, int32_t> read(std::span<std::byte> buffer) const noexcept;

    template<std::random_access_iterator It>
    [[nodiscard]] std::tuple<int32_t, int32_t> write(It first, std::size_t count) const noexcept {
        return write(std::as_bytes(std::span(first, count)));
    }

    template<std::random_access_iterator It, class End>
    [[nodiscard]] std::tuple<int32_t, int32_t> write(It first, End last) const noexcept {
        return write(std::as_bytes(std::span(first, last)));
    }

    template<class U, std::size_t N>
    [[nodiscard]] std::tuple<int32_t, int32_t> write(const std::array<U, N>& arr) const noexcept {
        return write(std::as_bytes(std::span(arr)));
    }

    [[nodiscard]] std::tuple<int32_t, int32_t> write(std::span<const std::byte> buffer) const noexcept;

    [[nodiscard]] virtual std::tuple<int32_t, int32_t> splice(const descriptor& source, std::size_t count, 
                                                              std::optional<int32_t> offset_in = std::nullopt,
                                                              std::optional<int32_t> offset_out = std::nullopt,
                                                              uint32_t flags = 0ul) const noexcept;

    template <typename Period>                                                              
    [[nodiscard]] int32_t select(duration<Period> &&timeout) const noexcept {
        return select(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count());
    }

    template <typename Period>
    [[nodiscard]] int32_t poll(duration<Period> &&timeout, polled_state &state) const noexcept {
        return poll(std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count(), state);
    }

    [[nodiscard]] static std::string_view error_description(int32_t error_code) noexcept;

protected:
    [[nodiscard]] int32_t select(int64_t timeout_nanoseconds) const noexcept;
    [[nodiscard]] int32_t poll(int64_t timeout_nanoseconds, polled_state &state) const noexcept;
};

// Helper functions
[[nodiscard]] int32_t unlink(std::string_view filename) noexcept;
}
#endif //DESCRIPTOR_DESCRIPTOR_H
