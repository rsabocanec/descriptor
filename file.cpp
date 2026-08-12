#include "file.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

int32_t rsabocanec::file::open(std::string_view path, int32_t flags, int32_t mode/* = 0*/) noexcept {
    descriptor_ = ::open(path.data(), flags, mode);

    if (descriptor_.load() == -1) {
        return errno;
    }

    return 0;
}

int64_t rsabocanec::file::size() const noexcept {
    if (descriptor_.load() == -1) {
        return -1;
    }

    struct stat st {};
    if (::fstat(descriptor_.load(), &st) == -1) {
        return -1;
    }

    return static_cast<int64_t>(st.st_size);
}

int32_t rsabocanec::file::truncate(std::size_t size) noexcept {
    if (descriptor_.load() == -1) {
        return -1;
    }

    auto const result = ::ftruncate(descriptor_.load(), static_cast<off_t>(size));
    if (result == -1) {
        return errno;
    }

    return 0;
}

int32_t rsabocanec::file::memory_map(memory_map_access access, std::size_t size/* = 0*/) noexcept {
    if (descriptor_.load() == -1) {
        return -1;
    }

    struct stat st {};
    if (::fstat(descriptor_.load(), &st) == -1) {
        return -1;
    }

    if (memory_map_addr_ != nullptr) {
        // Memory is already mapped
        auto const unmap_result = ::munmap(memory_map_addr_, memory_map_length_);
        if (unmap_result == -1) {
            return errno;
        }
    }

    memory_map_addr_ = nullptr;
    memory_map_length_ = 0;

    const std::size_t length = size == 0 ? static_cast<std::size_t>(st.st_size) : size;

    auto const result = ::mmap( nullptr, length, 
                                static_cast<int32_t>(access), 
                                MAP_SHARED, descriptor_.load(), 0);

    if (result == MAP_FAILED) {
        return errno;
    }

    memory_map_addr_ = result;
    memory_map_length_ = length;

    return 0;
}

int32_t rsabocanec::file::memory_unmap() noexcept {
    if (memory_map_addr_ == nullptr) {
        return 0; // Nothing to unmap
    }

    auto const result = ::munmap(memory_map_addr_, memory_map_length_);
    if (result == -1) {
        return errno;
    }

    memory_map_addr_ = nullptr;
    memory_map_length_ = 0;

    return 0;
}

int32_t rsabocanec::file::memory_sync(memory_map_sync_flags flags) noexcept {
    if (memory_map_addr_ == nullptr) {
        return 0; // Nothing to sync
    }

    auto const result = ::msync(memory_map_addr_, memory_map_length_, static_cast<int32_t>(flags));
    if (result == -1) {
        return errno;
    }

    return 0;
}

int32_t rsabocanec::file::memory_lock(std::size_t size/* = 0*/) noexcept {
    if (memory_map_addr_ == nullptr) {
        return 0; // Nothing to lock
    }

    const std::size_t length = size == 0 ? memory_map_length_ : size;

    auto const result = ::mlock2(memory_map_addr_, length, MLOCK_ONFAULT);
    if (result == -1) {
        return errno;
    }

    memory_lock_length_ = length;

    return 0;
}

int32_t rsabocanec::file::memory_unlock() noexcept {
    if (memory_map_addr_ == nullptr || memory_lock_length_ == 0) {
        return 0; // Nothing to unlock
    }

    auto const result = ::munlock(memory_map_addr_, memory_lock_length_);
    if (result == -1) {
        return errno;
    }

    memory_lock_length_ = 0;

    return 0;
}
