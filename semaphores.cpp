#include "semaphores.h"

#include <cerrno>

#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int32_t rsabocanec::semaphore::wait() noexcept {
    if (sem_ == nullptr) {
        return EINVAL;
    }
    else {
        if (::sem_wait(static_cast<sem_t*>(static_cast<void*>(sem_))) == -1) {
            return errno;
        }
    }

    return 0;
}

int32_t rsabocanec::semaphore::try_wait() noexcept {
    if (sem_ == nullptr) {
        return EINVAL;
    }
    else {
        if (::sem_trywait(static_cast<sem_t*>(static_cast<void*>(sem_))) == -1) {
            return errno;
        }
    }

    return 0;
}

int32_t rsabocanec::semaphore::post() noexcept {
    if (sem_ == nullptr) {
        return EINVAL;
    }
    else {
        if (::sem_post(static_cast<sem_t*>(static_cast<void*>(sem_))) == -1) {
            return errno;
        }
    }

    return 0;
}

int32_t rsabocanec::named_semaphore::open(std::string_view name, uint32_t initial_value/* = 0ul*/) noexcept {
    if (sem_ != nullptr) {
        if (::sem_close(static_cast<sem_t*>(static_cast<void*>(sem_))) == -1) {
            return errno;
        }        
    }

    sem_ = static_cast<long int*>(static_cast<void*>(
                ::sem_open(name.data(), O_CREAT, 
                           0644 /*S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH*/,
                           initial_value)));
        
    auto const result = sem_ == nullptr ? errno : 0;

    if (sem_ != nullptr) {
        name_ = name;
    }

    return result;
}

int32_t rsabocanec::named_semaphore::close() noexcept {
    if (sem_) {
        if (::sem_close(static_cast<sem_t*>(static_cast<void*>(sem_))) == -1) {
            return errno;
        }

        sem_ = nullptr;
    }

    if (::sem_unlink(name_.c_str()) == -1) {
        return errno;
    }

    name_.clear();

    return 0;
}

int32_t rsabocanec::unnamed_semaphore::init(std::string_view name, uint32_t initial_value) noexcept {
    if (auto const result = close(); result != 0) {
        return result;
    }

    name_ = name;

    int shared_semaphore = 0; // 0 for process-local, 1 for process-shared

    if (name_.empty()) {
        sem_ = static_cast<long int*>(static_cast<void*>(sem_));
    }
    else {
        // Create a semaphore in shared memory for process-shared semaphores
        shared_semaphore = 1;
        auto const fd = ::memfd_create(name_.c_str(), MFD_ALLOW_SEALING);
        if (fd == -1) {
            auto const result = errno;
            name_.clear();
            return result;
        }

        if (::ftruncate(fd, sizeof(sem_t)) == -1) {
            auto const result = errno;
            ::close(fd);
            name_.clear();
            return result;
        }

        sem_ = static_cast<long int*>(
                ::mmap(nullptr, sizeof(sem_t), PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd, 0));

        if (sem_ == MAP_FAILED) {
            auto const result = errno;
            ::close(fd);
            name_.clear();
            return result;
        }
    }
   

    if (::sem_init(static_cast<sem_t*>(static_cast<void*>(sem_)), shared_semaphore, initial_value) == -1) {
        return errno;
    }

    return 0;
}

int32_t rsabocanec::unnamed_semaphore::close() noexcept {
    if (sem_) {
        if (::sem_destroy(static_cast<sem_t*>(static_cast<void*>(sem_))) == -1) {
            return errno;
        }

        if (::sem_close(static_cast<sem_t*>(static_cast<void*>(sem_))) == -1) {
            return errno;
        }

        if (name_.empty()) {
            delete static_cast<sem_t*>(static_cast<void*>(sem_));
        }

        sem_ = nullptr;
    }

    name_.clear();

    return 0;
}