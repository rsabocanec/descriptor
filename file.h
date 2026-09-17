/*
Copyright (c) 2026 robert.sabocanec@gmail.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DESCRIPTOR_FILE_H
#define DESCRIPTOR_FILE_H

#pragma once

#include "descriptor.h"

namespace descriptor {

enum class memory_map_access : int32_t {
    read = 1,       //PROT_READ,
    write = 2,      // PROT_WRITE,
    readwrite = 3   // PROT_READ | PROT_WRITE
};

enum class memory_map_sync_flags : int32_t {
    async = 1,      // MS_ASYNC,
    sync = 2,       // MS_SYNC,
    invalidate = 4  // MS_INVALIDATE
};

class file : public descriptor {
    void* memory_map_addr_{nullptr};
    std::size_t memory_map_length_{0};
    std::size_t memory_lock_length_{0};

public:
    file() = default;
    file(const file&) = delete;
    file(file&&) = default;

    file& operator=(const file&) = delete;
    file& operator=(file&&) = default;

    virtual ~file() {
        [[maybe_unused]] auto result = memory_unlock();
        result = memory_sync(memory_map_sync_flags::sync);
        result = memory_unmap();
        result = close();
    }

    [[nodiscard]] virtual int32_t open( std::string_view path, 
                                        int32_t flags = 0102 /* O_RDWR | O_CREAT*/, 
                                        int32_t mode = 0644 /*S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH*/) noexcept;

    [[nodiscard]] int64_t size() const noexcept;

    [[nodiscard]] int32_t truncate(std::size_t size) noexcept;

    [[nodiscard]] int32_t deallocate(std::size_t size) noexcept {
        return truncate(size);
    }

    [[nodiscard]] int32_t memory_map(memory_map_access access, std::size_t size = 0) noexcept;
    [[nodiscard]] int32_t memory_unmap() noexcept;
    [[nodiscard]] int32_t memory_sync(memory_map_sync_flags flags) noexcept;
    [[nodiscard]] int32_t memory_lock(std::size_t size = 0) noexcept;
    [[nodiscard]] int32_t memory_unlock() noexcept;

    [[nodiscard]] void* memory_map_address() const noexcept {
        return memory_map_addr_;
    }

    [[nodiscard]] std::size_t memory_map_size() const noexcept {
        return memory_map_length_;
    }

    [[nodiscard]] std::size_t memory_lock_size() const noexcept {
        return memory_lock_length_;
    }

protected:    
    [[nodiscard]] int32_t create() noexcept final {
        return 0;
    }
};

class reader {
    file file_;
public:
    reader() = delete;
    explicit reader(std::string_view path) {
        [[maybe_unused]] auto const result = file_.open(path, 0/*O_RDONLY*/);
    }

    reader(const reader&) = delete;
    reader(reader&&) = default;

    reader& operator=(const reader&) = delete;
    reader& operator=(reader&&) = default;

    ~reader() = default;

    bool valid() const noexcept {
        return file_.valid();
    }
    
    template<std::random_access_iterator It>
    [[nodiscard]] std::tuple<int32_t, int32_t> read(It first, std::size_t count) const noexcept {
        return file_.read(first, count);
    }

    template<std::random_access_iterator It, class End>
    [[nodiscard]] std::tuple<int32_t, int32_t> read(It first, End last) const noexcept {
        return file_.read(first, last);
    }

    template<class U, std::size_t N>
    [[nodiscard]] std::tuple<int32_t, int32_t> read(std::array<U, N>& arr) const noexcept {
        return file_.read(arr);
    }

    [[nodiscard]] std::tuple<int32_t, int32_t> read(std::span<std::byte> buffer) const noexcept {
        return file_.read(buffer);
    }

    template <typename Period>
    [[nodiscard]] auto poll(duration<Period> &&timeout, polled_state &state) const noexcept {
        return file_.poll(std::forward<duration<Period>>(timeout), state);
    }
};

class writer {
    file file_;
public:
    writer() = delete;
    explicit writer(std::string_view path) {
        [[maybe_unused]] auto const result = file_.open(path, 0101 /*O_WRONLY | O_CREAT*/);
    }

    writer(const writer&) = delete;
    writer(writer&&) = default;

    writer& operator=(const writer&) = delete;
    writer& operator=(writer&&) = default;

    ~writer() = default;

    bool valid() const noexcept {
        return file_.valid();
    }
    
    template<std::random_access_iterator It>
    [[nodiscard]] std::tuple<int32_t, int32_t> write(It first, std::size_t count) const noexcept {
        return file_.write(first, count);
    }

    template<std::random_access_iterator It, class End>
    [[nodiscard]] std::tuple<int32_t, int32_t> write(It first, End last) const noexcept {
        return file_.write(first, last);
    }

    template<class U, std::size_t N>
    [[nodiscard]] std::tuple<int32_t, int32_t> write(const std::array<U, N>& arr) const noexcept {
        return file_.write(arr);
    }

    [[nodiscard]] std::tuple<int32_t, int32_t> write(std::span<const std::byte> buffer) const noexcept {
        return file_.write(buffer);
    }

    template <typename Period>
    [[nodiscard]] auto poll(duration<Period> &&timeout, polled_state &state) const noexcept {
        return file_.poll(std::forward<duration<Period>>(timeout), state);
    }
};
}

#endif // DESCRIPTOR_FILE_H