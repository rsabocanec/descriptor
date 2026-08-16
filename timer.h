#ifndef RSABOCANEC_TIMER_H
#define RSABOCANEC_TIMER_H

#pragma once

#include "descriptor.h"

#include <chrono>
#include <thread>
#include <future>
#include <tuple>
#include <utility>
#include <type_traits>
#include <optional>
#include <functional>

namespace rsabocanec {

template <typename Period>
using duration = std::chrono::duration<int64_t, Period>;

// Helper method for invoking a method in the timer class
template <class Tuple, size_t... Indices>
inline void invoke(Tuple&& func_values, std::index_sequence<Indices...>) noexcept{
    std::invoke(std::move(std::get<Indices>(func_values))...);
}

enum class clock_type : int {
    clock_realtime = 0,
    clock_monotonic = 1,
    //clock_process_cputime_id = 2,
    //clock_thread_cputime_id = 3,
    //clock_monotonic_raw = 4,
    //clock_realtime_coarse = 5,
    //clock_monotonic_coarse = 6,
    clock_boottime = 7,
    clock_realtime_alarm = 8,
    clock_boottime_alarm = 9
    //clock_tai = 11
};

class timer : public descriptor {
protected:
    clock_type clock_type_{clock_type::clock_monotonic};

    public:
    timer() noexcept {
        [[maybe_unused]] auto const result = create();
    }

    explicit timer(clock_type clock_type) noexcept
    : clock_type_(clock_type) {
        [[maybe_unused]] auto const result = create();
    }

    timer(const timer&) = delete;

    timer(timer&& other) noexcept
    : clock_type_(other.clock_type_) {
        const int32_t tmp = other.descriptor_.load();
        other.descriptor_ = -1;
        descriptor_ = tmp;

        clock_type_ = other.clock_type_;
    }

    timer& operator=(const timer&) = delete;

    timer& operator=(timer&& other) noexcept {
        if (this != &other) {
            const int32_t tmp = other.descriptor_.load();
            other.descriptor_ = -1;
            descriptor_ = tmp;

            clock_type_ = other.clock_type_;
        }

        return *this;
    }

    virtual ~timer() = default;

    template <typename Period>
    [[nodiscard]] int32_t arm(duration<Period>&& interval) const noexcept {
        return arm(std::chrono::duration_cast<std::chrono::nanoseconds>(interval).count());
    }

    template <typename Period, typename Function, typename... Args>
    void arm(duration<Period>&& interval,
             std::promise<int32_t>&& promise,
             Function&& func, Args&&... args) const noexcept {
        if (descriptor_ == -1) {
            promise.set_value(-1);
        }
        else {
            auto result = arm(std::forward<duration<Period>>(interval));

            if (result == 0) {
                using tuple = std::tuple<std::decay_t<Function>, std::decay_t<Args>...>;
                auto decay_copied = std::make_unique<tuple>(std::forward<Function>(func), std::forward<Args>(args)...);

                std::thread([this](std::promise<int32_t>&& p, decltype(decay_copied)&& params) {
                    int32_t result = 0;
                    while (!disarmed()) {
                        result = wait();
                        if (result == 0) {
                            invoke<tuple>(tuple(*params), std::make_index_sequence<1 + sizeof...(Args)>{});
                        }
                        else {
                            break;
                        }
                    }

                    p.set_value(result);
                }, std::forward<std::promise<int32_t>>(promise), std::move(decay_copied)).detach();
            }
            else {
                promise.set_value(result);
            }
        }
    }

    [[nodiscard]] int32_t disarm() const noexcept;
    [[nodiscard]] bool disarmed() const noexcept;
    [[nodiscard]] int32_t wait() const noexcept;

protected:
    [[nodiscard]] int32_t create() noexcept final override;

    [[nodiscard]] int32_t arm(int64_t nanoseconds) const noexcept;

    using timer_interval = std::tuple<int32_t, int32_t, int32_t, int32_t>;
    [[nodiscard]] virtual int32_t timer_spec(int64_t nanoseconds, timer_interval &interval) const noexcept;
};

class deadline : public timer {
public:
    deadline() = default;

    explicit deadline(clock_type clock_type) noexcept
    : timer(clock_type) {
    }

    deadline(const deadline&) = delete;
    deadline(deadline&& other) = default;

    deadline& operator=(const deadline&) = delete;
    deadline& operator=(deadline&& other) = default;

    ~deadline() override = default;

protected:
    [[nodiscard]] int32_t timer_spec(int64_t nanoseconds, timer_interval &interval) const noexcept final override;
};
}
#endif // RSABOCANEC_TIMER_H