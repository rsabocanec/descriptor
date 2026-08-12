#ifndef RSABOCANEC_FILE_H
#define RSABOCANEC_FILE_H

#pragma once

#include "descriptor.h"

namespace rsabocanec {

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

    [[nodiscard]] int32_t open( std::string_view path, 
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
};
}

#endif // RSABOCANEC_FILE_H