#include "file.h"

#include <fcntl.h>
#include <sys/stat.h>

int32_t rsabocanec::file::open(std::string_view path, int32_t flags, int32_t mode/* = 0*/) noexcept {
    descriptor_ = ::open(path.data(), flags, mode);

    if (descriptor_.load() == -1) {
        return errno;
    }

    return 0;
}

int64_t rsabocanec::file::size() const noexcept {
    if (descriptor_.load() == -1) {
        return -1;
    }

    struct stat st {};
    if (::fstat(descriptor_.load(), &st) == -1) {
        return -1;
    }

    return static_cast<int64_t>(st.st_size);
}