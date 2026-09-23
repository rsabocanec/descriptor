#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include "descriptor.h"

#include <ostream>

#ifdef _STACKTRACE_SUPPORTED
# include <stacktrace>
#endif

#include <cstring>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>
#include <signal.h>
#include <err.h>

#define _FILE_OFFSET_BITS 64
#include <unistd.h>

namespace descriptor {

void polled_state::set_state(int16_t revents) noexcept {
    has_something_to_read_ = revents & POLLIN;
    has_something_to_write_ = revents & POLLOUT;
    has_error_ = revents & POLLERR;
    has_hangup_ = revents & POLLHUP;
    has_peer_closed_ = revents & POLLRDHUP;
    has_invalid_request_ = revents & POLLNVAL;
    has_exception_ = revents & POLLPRI;
}

int32_t descriptor::close() noexcept {
    if (descriptor_.load() != -1) {
        if (::close(descriptor_.load()) == -1) {
            return errno;
        }

        descriptor_ = -1;
    }

    return 0;
}

std::tuple<int32_t, int32_t> descriptor::read(std::span<std::byte> buffer) const noexcept {
    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    if (descriptor_.load() != -1) {
        std::get<1>(result) =
            ::read(descriptor_.load(),
                    static_cast<void*>(buffer.data()),
                    static_cast<std::size_t>(buffer.size()));

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

std::tuple<int32_t, int32_t> descriptor::write(std::span<const std::byte> buffer) const noexcept {
    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    if (descriptor_.load() != -1) {
        std::get<1>(result) =
            ::write(descriptor_.load(),
                    static_cast<const void*>(buffer.data()),
                    static_cast<std::size_t>(buffer.size()));

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

std::tuple<int32_t, int32_t> descriptor::splice(const descriptor& source, std::size_t count,
                                                std::optional<int32_t> offset_in/* = std::nullopt*/,
                                                std::optional<int32_t> offset_out/* = std::nullopt*/,
                                                uint32_t flags/* = 0ul*/) const noexcept {

    std::tuple<int32_t, int32_t> result {EINVAL, -1};

    off_t offset_in_value = offset_in ? static_cast<off_t>(*offset_in) : 0;
    off_t offset_out_value = offset_out ? static_cast<off_t>(*offset_out) : 0;

    if (descriptor_.load() != -1) {
        std::get<1>(result) =
            ::copy_file_range(source.descriptor_.load(), 
                              offset_in ? &offset_in_value : nullptr, 
                              descriptor_.load(), 
                              offset_out ? &offset_out_value : nullptr, 
                              count, flags);

        std::get<0>(result) = std::get<1>(result) == -1 ? errno : 0;
    }

    return result;
}

int32_t descriptor::select(int64_t timeout_nanoseconds) const noexcept {
    if (descriptor_.load() == -1) {
        return -1;
    }

    sigset_t sigmask{};

    ::sigemptyset(&sigmask);
    ::sigaddset(&sigmask, SIGINT);
    ::sigaddset(&sigmask, SIGTERM);
    ::sigaddset(&sigmask, SIGHUP);
    ::sigaddset(&sigmask, SIGQUIT);
    ::sigaddset(&sigmask, SIGABRT);

    sigset_t oldmask{};

    const struct timespec ts {
        .tv_sec = static_cast<time_t>(timeout_nanoseconds / 1'000'000'000),
        .tv_nsec = static_cast<long>(timeout_nanoseconds % 1'000'000'000)
    };

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(descriptor_.load(), &rfds);

    ::pthread_sigmask(SIG_SETMASK, nullptr, &oldmask);
    
    int32_t ready{};

    pthread_cleanup_push([](void *arg) {
        ::pthread_sigmask(SIG_SETMASK, static_cast<sigset_t*>(arg), nullptr);
    }, &oldmask);

    ready = ::pselect(descriptor_.load() + 1, &rfds, nullptr, nullptr, &ts, &sigmask);

    if (ready == -1) {
        ready = errno;
    }

    pthread_cleanup_pop(0);

    return ready > 0 ? 0 : ETIMEDOUT;
}

int32_t descriptor::poll(int64_t timeout_nanoseconds, polled_state &state) const noexcept {
    state = {};

    if (descriptor_.load() == -1) {
        return -1;
    }

    sigset_t sigmask{};

    ::sigemptyset(&sigmask);
    ::sigaddset(&sigmask, SIGINT);
    ::sigaddset(&sigmask, SIGTERM);
    ::sigaddset(&sigmask, SIGHUP);
    ::sigaddset(&sigmask, SIGQUIT);
    ::sigaddset(&sigmask, SIGABRT);

    sigset_t oldmask{};

    const struct timespec ts {
        .tv_sec = static_cast<time_t>(timeout_nanoseconds / 1'000'000'000),
        .tv_nsec = static_cast<long>(timeout_nanoseconds % 1'000'000'000)
    };

    struct pollfd pfd{
        .fd = descriptor_.load(),
        .events = POLLIN | POLLERR | POLLHUP | POLLNVAL | POLLRDHUP | POLLPRI,
        .revents = 0
    };

    ::pthread_sigmask(SIG_SETMASK, nullptr, &oldmask);
    
    int32_t ready{};

    pthread_cleanup_push([](void *arg) {
        ::pthread_sigmask(SIG_SETMASK, static_cast<sigset_t*>(arg), nullptr);
    }, &oldmask);

    ready = ::ppoll(&pfd, 1, &ts, &sigmask);

    if (ready == -1) {
        ready = errno;
    }

    pthread_cleanup_pop(0);

    state.set_state(pfd.revents);

    return ready > 0 ? 0 : ETIMEDOUT;
}

// Utility functions
std::string_view error_description(int32_t error_code) noexcept {
    return ::strerror(error_code);
}

void report_error(std::ostream& os, int32_t error_num, std::string_view prefix,
                  std::source_location location) noexcept {
    os  << prefix << "Error " << error_num
        << '[' << ::strerror(error_num) << "]\t"
        << "in " << location.file_name()
        << " (" << location.function_name() << "), line "
        << location.line() << '\n'
        << "\nStack trace:\n" 
#ifdef _STACKTRACE_SUPPORTED        
        << std::stacktrace::current() << '\n'
#endif
        ;
}

int32_t unlink(std::string_view filename) noexcept {
    const int32_t result = ::unlink(filename.data());
    return result == -1 ? errno : 0;
}
}
