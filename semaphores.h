/*
Copyright (c) 2026 robert.sabocanec@gmail.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DESCRIPTOR_SEMAPHORES_H
#define DESCRIPTOR_SEMAPHORES_H

#pragma once

#include <string>
#include <string_view>
#include <atomic>
#include <system_error>

#include <semaphore.h>

namespace descriptor {
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

    [[nodiscard]] std::error_code wait() noexcept;
    [[nodiscard]] std::error_code try_wait() noexcept;

    [[nodiscard]] std::error_code post() noexcept;

    [[nodiscard]] virtual std::error_code open(std::string_view name, uint32_t initial_value = 0ul) noexcept = 0;
    [[nodiscard]] virtual std::error_code close() noexcept = 0;
};

class named_semaphore : public semaphore {
public:
    ~named_semaphore() {
        [[maybe_unused]] auto const result = close();
    }

    [[nodiscard]] std::error_code open(std::string_view name, uint32_t initial_value = 0ul) noexcept final override;
    [[nodiscard]] std::error_code close() noexcept final override;
};

class unnamed_semaphore : public semaphore {
public:
    ~unnamed_semaphore() {
        [[maybe_unused]] auto const result = close();
    }

    // name represents the name of the shared memory object that holds the semaphore, for process-shared semaphores
    // If name is nullptr, the semaphore is process-local and does not need to be named.
    [[nodiscard]] std::error_code open(std::string_view name, uint32_t initial_value = 0ul) noexcept final override {
        return init(name, initial_value);
    }

    [[nodiscard]] std::error_code close() noexcept final override;

protected:
    [[nodiscard]] std::error_code init(std::string_view name, uint32_t initial_value) noexcept;
};
}
#endif //DESCRIPTOR_SEMAPHORES_H