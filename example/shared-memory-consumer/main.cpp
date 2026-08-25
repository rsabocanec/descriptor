#include <shared_memory.h>
#include <semaphores.h>

#include <array>
#include <span>
#include <thread>
#include <chrono>
#include <iostream>

#include <fcntl.h>

auto main(int argc, char** argv) ->int {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <shared_memory filename>\n";
        return EXIT_FAILURE;
    }

    std::string_view filename{argv[1]};

    constexpr std::size_t shared_memory_size = 1024;

    rsabocanec::shared_memory shared_mem{};

    if (auto const open_result = shared_mem.open(filename, O_RDWR); open_result != 0) {
        std::cerr << "Failed to open shared memory " << filename << " with result " << open_result << ' '
                    << rsabocanec::descriptor::error_description(open_result) << '\n';
        return open_result;
    }

    std::cout << "Shared memory " << filename << " opened successfully\n";

    rsabocanec::named_semaphore semaphore{};

    std::string semaphore_name = std::string(filename) + "_semaphore";

    if (auto const open_result = semaphore.open(semaphore_name); open_result != 0) {
        std::cerr << "Failed to open semaphore " << semaphore_name << " with result " << open_result << ' '
                    << rsabocanec::descriptor::error_description(open_result) << '\n';
        return open_result;
    }

    std::cout << "Semaphore " << semaphore_name << " opened successfully\n";

    if (auto const post_result = semaphore.post(); post_result != 0) {
        std::cerr << "Failed to post on semaphore " << semaphore_name << " with result " << post_result << ' '
                << rsabocanec::descriptor::error_description(post_result) << '\n';
        return post_result;
    }

    std::array<char, shared_memory_size> buffer{};

    for (auto i = 0; i < 10; ++i) {
        std::cout << "Waiting for message " << i << " from shared memory " << filename << '\n';

        if (auto const wait_result = semaphore.wait(); wait_result != 0) {
            std::cerr << "Failed to wait on semaphore " << semaphore_name << " with result " << wait_result << ' '
                    << rsabocanec::descriptor::error_description(wait_result) << '\n';
            return wait_result;
        }

        std::cout << "Reading message " << i << " from shared memory " << filename << '\n';

        auto const [read_result, bytes_read] = shared_mem.read(buffer);
        if (read_result != 0) {
            std::cerr << "Failed to read from shared memory " << filename << " with result " << read_result << ' '
                    << rsabocanec::descriptor::error_description(read_result) << '\n';
        }
        else {
            std::cout.write(buffer.data(), bytes_read);
            std::cout << '\n';
        }

        if (auto const truncate_result = shared_mem.truncate(0); truncate_result != 0) {
            std::cerr << "Failed to truncate shared memory " << filename << " with result " << truncate_result << ' '
                    << rsabocanec::descriptor::error_description(truncate_result) << '\n';
        }

        std::cout << "Truncated shared memory " << filename << " to size 0\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (auto const post_result = semaphore.post(); post_result != 0) {
            std::cerr << "Failed to post on semaphore " << semaphore_name << " with result " << post_result << ' '
                    << rsabocanec::descriptor::error_description(post_result) << '\n';
            return post_result;
        }
    }

    return EXIT_SUCCESS;
}
