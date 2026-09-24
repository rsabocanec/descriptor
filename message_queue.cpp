#include "message_queue.h"

namespace descriptor {
int32_t message_queue::open( std::string_view path, int32_t flags, int32_t mode) noexcept {
    return -1;
}

int32_t message_queue::close() noexcept {
    return -1;
}

std::tuple<int32_t, int32_t> message_queue::read(std::span<std::byte> buffer) const noexcept {
    return {-1, -1};
}

std::tuple<int32_t, int32_t> message_queue::timed_read(std::span<std::byte> buffer, int64_t timeout_nanoseconds) const noexcept {
    return {-1, -1};
}

std::tuple<int32_t, int32_t> message_queue::write(std::span<const std::byte> buffer) const noexcept {
    return {-1, -1};
}

std::tuple<int32_t, int32_t> message_queue::timed_write(std::span<const std::byte> buffer, int64_t timeout_nanoseconds) const noexcept {
    return {-1, -1};
}

int32_t message_queue::notify() const noexcept {
    return -1;
}

int32_t message_queue::notify(int32_t signal_number) const noexcept {
    return -1;
}

int32_t message_queue::notify(std::function<void(void *ptr)> handler) const noexcept {
    return -1;
}
}