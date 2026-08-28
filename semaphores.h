#ifndef RSABOCANEC_SEMAPHORES_H
#define RSABOCANEC_SEMAPHORES_H

#pragma once

#include <string>
#include <string_view>
#include <atomic>

#include <semaphore.h>

namespace rsabocanec {
class semaphore {
protected:
    long int *sem_{nullptr};    
    std::string name_{};

public:
    semaphore() = default;
    semaphore(const semaphore&) = delete;
    semaphore(semaphore&&) = delete;

    semaphore& operator=(const semaphore&) = delete;
    semaphore& operator=(semaphore&&) = delete;

    virtual ~semaphore() = default;

    [[nodiscard]] int32_t wait() noexcept;
    [[nodiscard]] int32_t try_wait() noexcept;

    [[nodiscard]] int32_t post() noexcept;

    [[nodiscard]] virtual int32_t open(std::string_view name, uint32_t initial_value = 0ul) noexcept = 0;
    [[nodiscard]] virtual int32_t close() noexcept = 0;
};

class named_semaphore : public semaphore {
public:
    ~named_semaphore() {
        [[maybe_unused]] auto const result = close();
    }

    [[nodiscard]] int32_t open(std::string_view name, uint32_t initial_value = 0ul) noexcept final override;
    [[nodiscard]] int32_t close() noexcept final override;
};

class unnamed_semaphore : public semaphore {
public:
    ~unnamed_semaphore() {
        [[maybe_unused]] auto const result = close();
    }

    // name represents the name of the shared memory object that holds the semaphore, for process-shared semaphores
    // If name is nullptr, the semaphore is process-local and does not need to be named.
    [[nodiscard]] int32_t open(std::string_view name, uint32_t initial_value = 0ul) noexcept final override {
        return init(name, initial_value);
    }

    [[nodiscard]] int32_t close() noexcept final override;

protected:
    [[nodiscard]] int32_t init(std::string_view name, uint32_t initial_value) noexcept;
};
}
#endif //RSABOCANEC_SEMAPHORES_H