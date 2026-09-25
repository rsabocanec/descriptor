/*
Copyright (c) 2026 robert.sabocanec@gmail.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DESCRIPTOR_MESSAGE_QUEUE_H
#define DESCRIPTOR_MESSAGE_QUEUE_H

#pragma once

#include "file.h"

#include <functional>

namespace descriptor {
class message_queue : public basic_file {
    std::string name_;
    
public:
    message_queue() = default;
    message_queue(const message_queue&) = delete;
    message_queue(message_queue&&) = default;

    message_queue& operator=(const message_queue&) = delete;
    message_queue& operator=(message_queue&&) = default;

    [[nodiscard]] int32_t open( std::string_view name, 
                                int32_t flags = open_flags::read_write 
                                              | open_flags::create, 
                                int32_t mode  = open_mode::read_by_owner 
                                              | open_mode::write_by_owner
                                              | open_mode::read_by_group
                                              | open_mode::read_by_others) noexcept final override;

    [[nodiscard]] int32_t close() noexcept final override;


    [[nodiscard]] std::tuple<int32_t, int32_t> read(std::span<std::byte> buffer) const noexcept final override;

    template<std::random_access_iterator It, typename Period>
    [[nodiscard]] std::tuple<int32_t, int32_t> timed_read(It first, std::size_t count, duration<Period> &&timeout) const noexcept {
        return timed_read(std::as_writable_bytes(std::span(first, count)), std::forward<Period>(timeout));
    }

    template<std::random_access_iterator It, class End, typename Period>
    [[nodiscard]] std::tuple<int32_t, int32_t> timed_read(It first, End last, duration<Period> &&timeout) const noexcept {
        return timed_read(std::as_writable_bytes(std::span(first, last)), std::forward<Period>(timeout));
    }

    template<class U, std::size_t N, typename Period>
    [[nodiscard]] std::tuple<int32_t, int32_t> timed_read(std::array<U, N>& arr, duration<Period> &&timeout) const noexcept {
        return timed_read(std::as_writable_bytes(std::span(arr)), std::forward<Period>(timeout));
    }

    template <typename Period>
    [[nodiscard]] std::tuple<int32_t, int32_t> timed_read(std::span<std::byte> buffer, duration<Period> &&timeout) const noexcept {
        return timed_read(buffer, std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count());
    }

    [[nodiscard]] std::tuple<int32_t, int32_t> write(std::span<const std::byte> buffer) const noexcept final override;

    template<std::random_access_iterator It, typename Period>
    [[nodiscard]] std::tuple<int32_t, int32_t> timed_write(It first, std::size_t count, duration<Period> &&timeout) const noexcept {
        return timed_write(std::as_bytes(std::span(first, count)), std::forward<Period>(timeout));
    }

    template<std::random_access_iterator It, class End, typename Period>
    [[nodiscard]] std::tuple<int32_t, int32_t> timed_write(It first, End last, duration<Period> &&timeout) const noexcept {
        return timed_write(std::as_bytes(std::span(first, last)), std::forward<Period>(timeout));
    }

    template<class U, std::size_t N, typename Period>
    [[nodiscard]] std::tuple<int32_t, int32_t> timed_write(const std::array<U, N>& arr, duration<Period> &&timeout) const noexcept {
        return timed_write(std::as_bytes(std::span(arr)), std::forward<Period>(timeout));
    }

    template <typename Period>
    [[nodiscard]] std::tuple<int32_t, int32_t> timed_write(std::span<const std::byte> buffer, duration<Period> &&timeout) const noexcept {
        return timed_write(buffer, std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count());
    }

    int32_t notify() const noexcept;
    int32_t notify(int32_t signal_number) const noexcept;
    int32_t notify(std::function<void(void *ptr)> handler) const noexcept; 

protected:
    [[nodiscard]] std::tuple<int32_t, int32_t> timed_read(std::span<std::byte> buffer, int64_t timeout_nanoseconds) const noexcept;
    [[nodiscard]] std::tuple<int32_t, int32_t> timed_write(std::span<const std::byte> buffer, int64_t timeout_nanoseconds) const noexcept;
};
}
#endif // DESCRIPTOR_MESSAGE_QUEUE_H
