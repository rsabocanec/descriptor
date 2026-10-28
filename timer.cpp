#include "timer.h"

#include <cstdio>
#include <ctime>

#include <sys/timerfd.h>
#include <sys/types.h>

#include <unistd.h>

namespace descriptor {
static_assert(
    static_cast<int>(clock_type::clock_realtime) == CLOCK_REALTIME &&
    static_cast<int>(clock_type::clock_monotonic) == CLOCK_MONOTONIC &&
    static_cast<int>(clock_type::clock_boottime) == CLOCK_BOOTTIME &&
    static_cast<int>(clock_type::clock_realtime_alarm) == CLOCK_REALTIME_ALARM &&
    static_cast<int>(clock_type::clock_boottime_alarm) == CLOCK_BOOTTIME_ALARM
);

// timer implementation
std::error_code timer::disarm() const noexcept {
    if (!valid()) {
        return std::make_error_code(std::errc::bad_file_descriptor);
    }

    constexpr struct itimerspec its = {
        .it_interval = {.tv_sec = 0, .tv_nsec = 0},
        .it_value = {.tv_sec = 0, .tv_nsec = 0}};

    // Acquire a mutex
    const std::lock_guard<std::mutex> lock(wait_mutex_);
    if (::timerfd_settime(descriptor_, TFD_TIMER_ABSTIME, &its, nullptr) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    return {};
}

bool timer::disarmed() const noexcept {
    if (valid()) {
        struct itimerspec period{};
        if (::timerfd_gettime(descriptor_, &period) != -1) {
            if (period.it_value.tv_sec != 0 || period.it_value.tv_nsec != 0) {
                return false;
            }
        }
    }

    return true;
}

std::error_code timer::wait() const noexcept {
    if (!valid()) {
        return std::make_error_code(std::errc::bad_file_descriptor);
    }

    uint64_t exp{};

    // Acquire a mutex
    const std::lock_guard<std::mutex> lock(wait_mutex_);
    switch (::read(descriptor_, &exp, sizeof(exp))) {
        case -1:
            return std::make_error_code(static_cast<std::errc>(errno));
        case sizeof(exp):
            return std::error_code{};
        default:
            return std::make_error_code(std::errc::value_too_large);
    }
}

std::error_code timer::create() noexcept {
    if (auto const result = close()) {
        return result;
    }

    descriptor_ = ::timerfd_create(static_cast<int>(clock_type_), 0);
    return descriptor_ == -1 ? std::make_error_code(static_cast<std::errc>(errno)) : std::error_code{};
}

std::error_code timer::arm(int64_t nanoseconds) const noexcept {
    if (nanoseconds < 0) {
        return std::make_error_code(std::errc::invalid_argument);
    }

    if (nanoseconds == 0) {
        return {};
    }

    if (!valid()) {
        return std::make_error_code(std::errc::bad_file_descriptor);
    }

    timer_interval interval{};

    if (auto const result = timer_spec(nanoseconds, interval)) {
        return result;
    }

    const struct itimerspec its = {
        .it_interval = {.tv_sec = std::get<0>(interval), .tv_nsec = std::get<1>(interval)},
        .it_value = {.tv_sec = std::get<2>(interval), .tv_nsec = std::get<3>(interval)}
    };

    // Acquire a mutex
    const std::lock_guard<std::mutex> lock(wait_mutex_);
    if (::timerfd_settime(descriptor_, TFD_TIMER_ABSTIME, &its, nullptr) == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    return {};
}

std::error_code timer::timer_spec(int64_t nanoseconds, timer_interval &interval) const noexcept {
    struct timespec now{};

    if (::clock_gettime(static_cast<int>(clock_type_), &now) != 0) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }
    else {
        interval = {
            nanoseconds / 1'000'000'000,
            nanoseconds % 1'000'000'000,
            now.tv_sec + nanoseconds / 1'000'000'000,
            now.tv_nsec
        };
    }

    return {};
}

std::error_code deadline::timer_spec(int64_t nanoseconds, timer_interval &interval) const noexcept {
    struct timespec now{};

    if (::clock_gettime(static_cast<int>(clock_type_), &now) != 0) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }
    else {
        interval = {
            0,
            0,
            now.tv_sec + nanoseconds / 1'000'000'000,
            now.tv_nsec
        };
    }

    return {};
}
}