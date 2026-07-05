#ifndef RSABOCANEC_FILE_H
#define RSABOCANEC_FILE_H

#pragma once

#include "descriptor.h"

namespace rsabocanec {
class file : public descriptor {
public:
    file() = default;

    [[nodiscard]] int32_t open(std::string_view path, int32_t flags, int32_t mode = 0) noexcept;

    [[nodiscard]] int64_t size() const noexcept;

protected:    
    [[nodiscard]] int32_t create() noexcept final {
        return 0;
    }
};
}

#endif // RSABOCANEC_FILE_H