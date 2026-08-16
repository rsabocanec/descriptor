#include "timer.h"

#include <cstdio>
#include <ctime>

#include <sys/timerfd.h>
#include <sys/types.h>

#include <unistd.h>

namespace rsabocanec {
static_assert(
    static_cast<int>(clock_type::clock_realtime) == CLOCK_REALTIME &&
    static_cast<int>(clock_type::clock_monotonic) == CLOCK_MONOTONIC &&
    static_cast<int>(clock_type::clock_boottime) == CLOCK_BOOTTIME &&
    static_cast<int>(clock_type::clock_realtime_alarm) == CLOCK_REALTIME_ALARM &&
    static_cast<int>(clock_type::clock_boottime_alarm) == CLOCK_BOOTTIME_ALARM
);

// timer implementation
int32_t timer::disarm() const noexcept {
    if (!valid()) {
        return -1;
    }

    constexpr struct itimerspec its = {
        .it_interval = {.tv_sec = 0, .tv_nsec = 0},
        .it_value = {.tv_sec = 0, .tv_nsec = 0}};

    if (::timerfd_settime(descriptor_, TFD_TIMER_ABSTIME, &its, nullptr) == -1) {
        return errno;
    }

    return 0;
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

int32_t timer::wait() const noexcept {
    if (!valid()) {
        return -1;
    }

    uint64_t exp{};

    switch (::read(descriptor_, &exp, sizeof(exp))) {
        case -1:
            return errno;
        case sizeof(exp):
            return 0;
        default:
            return EOVERFLOW;
    }
}

int32_t timer::create() noexcept {
    auto const result = close();
    if (result != 0) {
        return result;
    }

    descriptor_ = ::timerfd_create(static_cast<int>(clock_type_), 0);
    return descriptor_ == -1 ? errno : 0;
}

int32_t timer::arm(int64_t nanoseconds) const noexcept {
    if (nanoseconds < 0) {
        return EINVAL;
    }

    if (nanoseconds == 0) {
        return 0;
    }

    if (!valid()) {
        return -1;
    }

    timer_interval interval{};

    auto const result = timer_spec(nanoseconds, interval);
    if (result != 0) {
        return result;
    }

    const struct itimerspec its = {
        .it_interval = {.tv_sec = std::get<0>(interval), .tv_nsec = std::get<1>(interval)},
        .it_value = {.tv_sec = std::get<2>(interval), .tv_nsec = std::get<3>(interval)}
    };

    if (::timerfd_settime(descriptor_, TFD_TIMER_ABSTIME, &its, nullptr) == -1) {
        return errno;
    }

    return 0;
}

int32_t timer::timer_spec(int64_t nanoseconds, timer_interval &interval) const noexcept {
    struct timespec now{};

    if (::clock_gettime(static_cast<int>(clock_type_), &now) != 0) {
        return errno;
    }
    else {
        interval = {
            nanoseconds / 1'000'000'000,
            nanoseconds % 1'000'000'000,
            now.tv_sec + nanoseconds / 1'000'000'000,
            now.tv_nsec
        };
    }

    return 0;
}

int32_t deadline::timer_spec(int64_t nanoseconds, timer_interval &interval) const noexcept {
    struct timespec now{};

    if (::clock_gettime(static_cast<int>(clock_type_), &now) != 0) {
        return errno;
    }
    else {
        interval = {
            0,
            0,
            now.tv_sec + nanoseconds / 1'000'000'000,
            now.tv_nsec
        };
    }

    return 0;
}
}