#include <shared_memory.h>
#include <semaphores.h>

#include "../utility.hpp"

#include <array>
#include <span>
#include <thread>
#include <chrono>

#include <fcntl.h>

auto main(int argc, char** argv) ->int {

    const std::string shmem_filename_option{"-s,--shmem-filename"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {shmem_filename_option}, "Test shared memory consumer");

    const std::string shmem_filename = app->get_option(shmem_filename_option)->as<std::string>();

    constexpr std::size_t shared_memory_size = 1024;

    descriptor::shared_memory shared_mem{};

    if (auto const open_result = shared_mem.open(shmem_filename, O_RDWR); open_result != 0) {
        logger->error("Failed to open shared memory {}, with error {} '{}'", 
            shmem_filename, open_result, descriptor::descriptor::error_description(open_result));
        return open_result;
    }

    logger->info("Shared memory {} opened successfully", shmem_filename);

    descriptor::named_semaphore semaphore{};

    const std::string semaphore_name = "/" + std::string(shmem_filename) + "_semaphore";

    if (auto const open_result = semaphore.open(semaphore_name); open_result != 0) {
        logger->error("Failed to open semaphore {}, with error {} '{}'", 
            semaphore_name, open_result, descriptor::descriptor::error_description(open_result));
        return open_result;
    }

    logger->info("Semaphore {} opened successfully", semaphore_name);

    if (auto const post_result = semaphore.post(); post_result != 0) {
        logger->error("Failed to post on semaphore {}, with error {} '{}'", 
            semaphore_name, post_result, descriptor::descriptor::error_description(post_result));
        return post_result;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::array<char, shared_memory_size> buffer{};

    for (auto i = 0; i < 10; ++i) {
        logger->info("Waiting on semaphore {} from producer", semaphore_name);

        if (auto const wait_result = semaphore.wait(); wait_result != 0) {
            logger->error("Failed to wait on semaphore {}: {}", 
                semaphore_name, descriptor::descriptor::error_description(wait_result));
            return wait_result;
        }

        logger->info("Reading message {} from shared memory {}", i, shmem_filename);

        auto const [read_result, bytes_read] = shared_mem.read(buffer);
        if (read_result != 0) {
            logger->error("Failed to read from shared memory {}: {}", 
                shmem_filename, descriptor::descriptor::error_description(read_result));
        }
        else {
            if (bytes_read == 0) {
                logger->info("No data read from shared memory {}", shmem_filename);
            }
            else {
                fmt::print(fg(fmt::color::orange), "{}\n", std::string_view(buffer.data(), bytes_read));
                logger->info("Read {} bytes from shared memory {}", bytes_read, shmem_filename);

                if (auto const truncate_result = shared_mem.truncate(0); truncate_result != 0) {
                    logger->error("Failed to truncate shared memory {}: {}", 
                        shmem_filename, descriptor::descriptor::error_description(truncate_result));
                }
            }
        }

        if (auto const post_result = semaphore.post(); post_result != 0) {
            logger->error("Failed to post on semaphore {}: {}", 
                semaphore_name, descriptor::descriptor::error_description(post_result));
            return post_result;
        }

        logger->info("Posted on semaphore {} to signal producer", semaphore_name);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return EXIT_SUCCESS;
}
