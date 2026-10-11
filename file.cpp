#include "file.h"

#ifndef _GNU_SOURCE
# sdefine _GNU_SOURCE
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <linux/memfd.h>

// Check constants
static_assert(O_RDONLY == descriptor::open_flags::read_only, "O_RDONLY does not match descriptor::open_flags::read_only");
static_assert(O_WRONLY == descriptor::open_flags::write_only, "O_WRONLY does not match descriptor::open_flags::write_only");
static_assert(O_RDWR == descriptor::open_flags::read_write, "O_RDWR does not match descriptor::open_flags::read_write");
static_assert(O_CREAT == descriptor::open_flags::create, "O_CREAT does not match descriptor::open_flags::create");
static_assert(O_EXCL == descriptor::open_flags::exclusive, "O_EXCL does not match descriptor::open_flags::exclusive");
static_assert(O_TRUNC == descriptor::open_flags::truncate, "O_TRUC does not match descriptor::open_flags::truncate");
static_assert(O_APPEND == descriptor::open_flags::append, "O_APPEND does not match descriptor::open_flags::append");
static_assert(O_NONBLOCK == descriptor::open_flags::nonblocking, "O_NONBLOCK does not match descriptor::open_flags::nonblocking");


static_assert(descriptor::open_mode::read_by_owner == S_IRUSR, "S_IRUSR does not match descriptor::open_mode::read_by_owner");
static_assert(descriptor::open_mode::write_by_owner == S_IWUSR, "S_IWUSR does not match descriptor::open_mode::read_by_owner");
static_assert(descriptor::open_mode::exec_by_owner == S_IXUSR, "S_IXUSR does not match descriptor::open_mode::read_by_owner");
static_assert(descriptor::open_mode::all_by_owner == S_IRWXU, "S_IRWXU does not match descriptor::open_mode::read_by_owner");

static_assert(descriptor::open_mode::read_by_group == S_IRGRP, "S_IRGRP does not match descriptor::open_mode::read_by_group");
static_assert(descriptor::open_mode::write_by_group == S_IWGRP, "S_IWGRP does not match descriptor::open_mode::write_by_group");
static_assert(descriptor::open_mode::exec_by_group == S_IXGRP, "S_IXGRP does not match descriptor::open_mode::exec_by_group");
static_assert(descriptor::open_mode::all_by_group == S_IRWXG, "S_IRWXG does not match descriptor::open_mode::all_by_group");

static_assert(descriptor::open_mode::read_by_others == S_IROTH, "S_IROTH does not match descriptor::open_mode::read_by_others");
static_assert(descriptor::open_mode::write_by_others == S_IWOTH, "S_IWOTH does not match descriptor::open_mode::write_by_others");
static_assert(descriptor::open_mode::exec_by_others == S_IXOTH, "S_IXOTH does not match descriptor::open_mode::exec_by_others");
static_assert(descriptor::open_mode::all_by_others == S_IRWXO, "S_IRWXO does not match descriptor::open_mode::all_by_others");

static_assert(static_cast<int32_t>(descriptor::seek_position::set) == SEEK_SET, "SEEK_SET does not match descriptor::seek_position::set");
static_assert(static_cast<int32_t>(descriptor::seek_position::current) == SEEK_CUR, "SEEK_CUR does not match descriptor::seek_position::current");
static_assert(static_cast<int32_t>(descriptor::seek_position::end) == SEEK_END, "SEEK_END does not match descriptor::seek_position::end");
static_assert(static_cast<int32_t>(descriptor::seek_position::data) == SEEK_DATA, "SEEK_DATA does not match descriptor::seek_position::data");
static_assert(static_cast<int32_t>(descriptor::seek_position::hole) == SEEK_HOLE, "SEEK_HOLE does not match descriptor::seek_position::hole");
static_assert(static_cast<int32_t>(descriptor::memory_map_access::read) == PROT_READ, "PROT_READ does not match descriptor::memory_map_access::read");
static_assert(static_cast<int32_t>(descriptor::memory_map_access::write) == PROT_WRITE, "PROT_WRITE does not match descriptor::memory_map_access::write");
static_assert(static_cast<int32_t>(descriptor::memory_map_access::readwrite) == (PROT_READ | PROT_WRITE), "PROT_READ | PROT_WRITE does not match descriptor::memory_map_access::readwrite");
static_assert(static_cast<int32_t>(descriptor::memory_map_sync_flags::async) == MS_ASYNC, "MS_ASYNC does not match descriptor::memory_map_sync_flags::async");
static_assert(static_cast<int32_t>(descriptor::memory_map_sync_flags::sync) == MS_SYNC, "MS_SYNC does not match descriptor::memory_map_sync_flags::sync");
static_assert(static_cast<int32_t>(descriptor::memory_map_sync_flags::invalidate) == MS_INVALIDATE, "MS_INVALIDATE does not match descriptor::memory_map_sync_flags::invalidate");


int32_t descriptor::basic_file::open(std::string_view path, int32_t flags, int32_t mode/* = 0*/) noexcept {
    descriptor_ = ::open(path.data(), flags, mode);

    if (descriptor_.load() == -1) {
        return errno;
    }

    return 0;
}

int64_t descriptor::basic_file::size() const noexcept {
    if (descriptor_.load() == -1) {
        return -1;
    }

    struct stat st {};
    if (::fstat(descriptor_.load(), &st) == -1) {
        return -1;
    }

    return static_cast<int64_t>(st.st_size);
}

int32_t descriptor::basic_file::truncate(std::size_t size) const noexcept {
    if (descriptor_.load() == -1) {
        return -1;
    }

    auto const result = ::ftruncate(descriptor_.load(), static_cast<off_t>(size));
    if (result == -1) {
        return errno;
    }

    return 0;
}

int32_t descriptor::basic_file::seek(seek_position position, int64_t offset, int64_t &new_offset) const noexcept {
    new_offset = 0;

    if (descriptor_.load() == -1) {
        return EINVAL;
    }

    auto const result = ::lseek(descriptor_.load(), static_cast<off_t>(offset), static_cast<int32_t>(position));
    if (result == -1) {
        return errno;
    }

    new_offset = static_cast<int64_t>(result);
    return 0;
}

int32_t descriptor::file::memory_map(memory_map_access access, std::size_t size/* = 0644*/) noexcept {
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

    if (st.st_size >= 0 && size > static_cast<std::size_t>(st.st_size)) {
        // If the requested size is greater than the current file size, truncate the file
        auto const truncate_result = ::ftruncate(descriptor_.load(), static_cast<off_t>(size));
        if (truncate_result == -1) {
            return errno;
        }
    }

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

int32_t descriptor::file::memory_unmap() noexcept {
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

int32_t descriptor::file::memory_sync(memory_map_sync_flags flags) noexcept {
    if (memory_map_addr_ == nullptr) {
        return 0; // Nothing to sync
    }

    auto const result = ::msync(memory_map_addr_, memory_map_length_, static_cast<int32_t>(flags));
    if (result == -1) {
        return errno;
    }

    return 0;
}

int32_t descriptor::file::memory_lock(std::size_t size/* = 0*/) noexcept {
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

int32_t descriptor::file::memory_unlock() noexcept {
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


static_assert(MFD_CLOEXEC == descriptor::memory_file_flags::close_on_exec, "MFD_CLOEXEC does not match descriptor::memory_file_flags::close_on_exec");
static_assert(MFD_ALLOW_SEALING == descriptor::memory_file_flags::allow_sealing, "MFD_ALLOW_SEALING does not match descriptor::memory_file_flags::allow_sealing");
static_assert(MFD_HUGETLB == descriptor::memory_file_flags::huge_table, "MFD_HUGE_TABLE does not match descriptor::memory_file_flags::huge_table");

static_assert(descriptor::memory_file_flags::huge_64KB	== HUGETLB_FLAG_ENCODE_64KB ,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");
static_assert(descriptor::memory_file_flags::huge_512KB	== HUGETLB_FLAG_ENCODE_512KB,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");
static_assert(descriptor::memory_file_flags::huge_1MB	== HUGETLB_FLAG_ENCODE_1MB  ,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");
static_assert(descriptor::memory_file_flags::huge_2MB	== HUGETLB_FLAG_ENCODE_2MB  ,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");
static_assert(descriptor::memory_file_flags::huge_8MB	== HUGETLB_FLAG_ENCODE_8MB  ,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");
static_assert(descriptor::memory_file_flags::huge_16MB	== HUGETLB_FLAG_ENCODE_16MB ,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");
static_assert(descriptor::memory_file_flags::huge_32MB	== HUGETLB_FLAG_ENCODE_32MB ,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");
static_assert(descriptor::memory_file_flags::huge_256MB	== HUGETLB_FLAG_ENCODE_256MB,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");
static_assert(descriptor::memory_file_flags::huge_512MB	== HUGETLB_FLAG_ENCODE_512MB,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");
static_assert(descriptor::memory_file_flags::huge_1GB	== HUGETLB_FLAG_ENCODE_1GB  ,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");
static_assert(descriptor::memory_file_flags::huge_2GB	== HUGETLB_FLAG_ENCODE_2GB  ,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");
static_assert(descriptor::memory_file_flags::huge_16GB	== HUGETLB_FLAG_ENCODE_16GB ,
              "descriptor::memory_file_flags::huge_64KB does not match HUGETLB_FLAG_ENCODE_64KB");


int32_t descriptor::memory_file::open(std::string_view path, int32_t flags, int32_t) noexcept {
    descriptor_ = ::memfd_create(path.data(), static_cast<unsigned int>(flags));

    if (descriptor_.load() == -1) {
        return errno;
    }

    return 0;
}

// Utility functions
int32_t descriptor::make_fifo(std::string_view filename, int32_t mode) noexcept {
    const int32_t result = ::mkfifo(filename.data(), mode);
    return result == -1 ? errno : 0;
}
