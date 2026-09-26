#include "message_queue.h"

#include <fcntl.h>
#include <sys/stat.h> 
#include <mqueue.h>
#include <signal.h>
#include <limits.h>

namespace descriptor {
int32_t message_queue::open( std::string_view name, int32_t flags, int32_t mode) noexcept {
    if (descriptor_.load() != -1) {
        if (name == name_) {
            return 0; // Already opened
        }
        else if (auto const result = close(); result != 0) {
            return result;
        }
    }

    if (!name.starts_with('/')) {
        name_ = "/";
        name_ += name;
    }
    else {
        name_ = name;
    }

    if (name_.length() > NAME_MAX) {
        name_ = name_.substr(0, NAME_MAX);
    }

    descriptor_ = ::mq_open(name_.c_str(), flags, mode);

    if (descriptor_.load() == -1) {
        auto const result = errno;
        name_.clear();
        return result;
    }

    return 0;
}

int32_t message_queue::close() noexcept {
    if (descriptor_ != -1) {
        if (::mq_close(descriptor_) == -1) {
            return errno;
        }
    }

    if (!name_.empty()) {
        if (::mq_unlink(name_.c_str()) == -1) {
            return errno;
        }
    }

    return 0;
}

std::tuple<int32_t, int32_t> message_queue::read(std::span<std::byte> buffer) const noexcept {

    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    if (descriptor_.load() != -1) {
        [[maybe_unused]] unsigned int priority{};
        std::get<1>(result) =
            ::mq_receive(descriptor_.load(),
                    static_cast<char*>(static_cast<void*>(buffer.data())),
                    static_cast<std::size_t>(buffer.size()),
                    &priority);

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

std::tuple<int32_t, int32_t> message_queue::timed_read(std::span<std::byte> buffer, int64_t timeout_nanoseconds) const noexcept {

    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    if (descriptor_.load() != -1) {
        [[maybe_unused]] unsigned int priority{};

        struct timespec tm{};
        ::clock_gettime(CLOCK_REALTIME, &tm);
        tm.tv_sec += timeout_nanoseconds / 1'000'000'000;
        tm.tv_nsec += timeout_nanoseconds % 1'000'000'000;
    
        std::get<1>(result) =
            ::mq_timedreceive(descriptor_.load(),
                    static_cast<char*>(static_cast<void*>(buffer.data())),
                    static_cast<std::size_t>(buffer.size()),
                    &priority, &tm);

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

std::tuple<int32_t, int32_t> message_queue::write(std::span<const std::byte> buffer) const noexcept {

    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    if (descriptor_.load() != -1) {
        [[maybe_unused]] const unsigned int priority{};
        
        std::get<1>(result) =
            ::mq_send(descriptor_.load(),
                      static_cast<const char*>(static_cast<const void*>(buffer.data())),
                      static_cast<std::size_t>(buffer.size()),
                      priority);

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

std::tuple<int32_t, int32_t> message_queue::timed_write(std::span<const std::byte> buffer, int64_t timeout_nanoseconds) const noexcept {

    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    if (descriptor_.load() != -1) {
        [[maybe_unused]] const unsigned int priority{};

        struct timespec tm{};
        ::clock_gettime(CLOCK_REALTIME, &tm);
        tm.tv_sec += timeout_nanoseconds / 1'000'000'000;
        tm.tv_nsec += timeout_nanoseconds % 1'000'000'000;

        std::get<1>(result) =
            ::mq_timedsend(descriptor_.load(),
                           static_cast<const char*>(static_cast<const void*>(buffer.data())),
                           static_cast<std::size_t>(buffer.size()),
                           priority, &tm);

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

int32_t message_queue::notify() const noexcept {
    if (descriptor_ == -1) {
        return EINVAL;
    }

    const struct sigevent se {
        .sigev_notify = SIGEV_NONE
    };

    if (::mq_notify(descriptor_, &se) == -1) {
        return errno;
    }

    return 0;
}

int32_t message_queue::notify(int32_t signal_number) const noexcept {
    if (descriptor_ == -1) {
        return EINVAL;
    }

    const struct sigevent se {
        .sigev_signo = signal_number,
        .sigev_notify = SIGEV_SIGNAL
    };

    if (::mq_notify(descriptor_, &se) == -1) {
        return errno;
    }

    return 0;
}

int32_t message_queue::notify(std::function<void(void *ptr)> handler) const noexcept {
    if (descriptor_ == -1) {
        return EINVAL;
    }

    auto mqdesc = descriptor_.load();

    const union sigval sv {
        .sival_ptr = static_cast<void*>(&mqdesc)
    };

    static std::function<void(void *ptr)> sig_handler;
    sig_handler = std::move(handler);

    struct sigevent se {};
    se.sigev_value = sv;
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_notify_function = [](union sigval svt) { sig_handler(svt.sival_ptr); };
    se.sigev_notify_attributes = nullptr;

    if (::mq_notify(descriptor_, &se) == -1) {
        return errno;
    }

    return 0;
}
}