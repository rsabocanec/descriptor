#ifndef RSABOCANEC_SHARED_MEMORY_H
#define RSABOCANEC_SHARED_MEMORY_H

#pragma once

namespace rsabocanec {
class shared_memory {
public:
    shared_memory() = default;
    shared_memory(const shared_memory&) = delete;
    shared_memory(shared_memory&&) = delete;
    shared_memory& operator=(const shared_memory&) = delete;
    shared_memory& operator=(shared_memory&&) = delete;
    virtual ~shared_memory() = default;
};
}
#endif //RSABOCANEC_SHARED_MEMORY_H