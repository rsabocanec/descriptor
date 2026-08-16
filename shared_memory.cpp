#include "shared_memory.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

int32_t rsabocanec::shared_memory::open(std::string_view path, int32_t flags, int32_t mode/* = 0*/) noexcept {
    descriptor_ = ::shm_open(path.data(), flags, mode);

    if (descriptor_.load() == -1) {
        return errno;
    }

    path_ = path;

    return 0;
}

int32_t rsabocanec::shared_memory::close() noexcept {
    int32_t result = file::close();

    if (!path_.empty()) {
        auto const unlink_result = ::shm_unlink(path_.c_str());
        if (unlink_result == -1) {
            return errno;
        }
 
        path_.clear();
    }

    return result;
}