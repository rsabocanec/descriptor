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

    [[nodiscard]] int32_t open(std::string_view path, int32_t flags, int32_t mode = 0) noexcept;

    [[nodiscard]] int64_t size() const noexcept;

    [[nodiscard]] int32_t truncate(std::size_t size) noexcept;

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
}

#endif // RSABOCANEC_FILE_H