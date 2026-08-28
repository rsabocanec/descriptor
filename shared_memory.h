#ifndef RSABOCANEC_SHARED_MEMORY_H
#define RSABOCANEC_SHARED_MEMORY_H

#pragma once

#include "file.h"

namespace rsabocanec {
class shared_memory : public file {
    std::string path_; // Need to store the path for unlinking the shared memory object
public:
    shared_memory() = default;
    shared_memory(const shared_memory&) = delete;
    shared_memory(shared_memory&&) = delete;
    shared_memory& operator=(const shared_memory&) = delete;
    shared_memory& operator=(shared_memory&&) = delete;
    virtual ~shared_memory() = default;

    [[nodiscard]] int32_t open( std::string_view path, 
                                int32_t flags = 0102 /* O_RDWR | O_CREAT*/, 
                                int32_t mode = 0644 /*S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH*/) noexcept final override;

    [[nodiscard]] int32_t close() noexcept final override;
};
}
#endif //RSABOCANEC_SHARED_MEMORY_H