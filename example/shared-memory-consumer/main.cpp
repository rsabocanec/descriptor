#include <shared_memory.h>
#include <semaphores.h>

#include "../utility.hpp"

#include <array>
#include <span>
#include <thread>
#include <chrono>


auto main(int argc, char** argv) ->int {

    const std::string shmem_filename_option{"-s,--shmem-filename"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {shmem_filename_option}, "Test shared memory consumer");

    std::string shmem_filename{};

    try {
        shmem_filename = app->get_option("--shmem-filename")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    constexpr std::size_t shared_memory_size = 1024;

    descriptor::shared_memory shared_mem{};

    if (auto const open_result = shared_mem.open(shmem_filename, descriptor::open_flags::read_write)) {
        logger->error("Failed to open shared memory {}, with error {} '{}'", 
            shmem_filename, open_result.value(), open_result.message());
        return open_result.value();
    }

    logger->info("Shared memory {} opened successfully", shmem_filename);

    descriptor::named_semaphore semaphore{};

    const std::string semaphore_name = "/" + std::string(shmem_filename) + "_semaphore";

    if (auto const open_result = semaphore.open(semaphore_name)) {
        logger->error("Failed to open semaphore {}, with error {} '{}'", 
            semaphore_name, open_result.value(), open_result.message());
        return open_result.value();
    }

    logger->info("Semaphore {} opened successfully", semaphore_name);

    if (auto const post_result = semaphore.post()) {
        logger->error("Failed to post on semaphore {}, with error {} '{}'", 
            semaphore_name, post_result.value(), post_result.message());
        return post_result.value();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::array<char, shared_memory_size> buffer{};

    for (auto i = 0; i < 10; ++i) {
        logger->info("Waiting on semaphore {} from producer", semaphore_name);

        if (auto const wait_result = semaphore.wait()) {
            logger->error("Failed to wait on semaphore {}: {}", 
                semaphore_name, wait_result.message());
            return wait_result.value();
        }

        logger->info("Reading message {} from shared memory {}", i, shmem_filename);

        auto const read_result = shared_mem.read(buffer);
        if (!read_result) {
            logger->error("Failed to read from shared memory {}: {}", 
                shmem_filename, read_result.error().message());
        }
        else {
            if (read_result.value() == 0) {
                logger->info("No data read from shared memory {}", shmem_filename);
            }
            else {
                fmt::print(fg(fmt::color::orange), "{}\n", std::string_view(buffer.data(), read_result.value()));
                logger->info("Read {} bytes from shared memory {}", read_result.value(), shmem_filename);

                if (auto const truncate_result = shared_mem.truncate(0)) {
                    logger->error("Failed to truncate shared memory {}: {}", 
                        shmem_filename, truncate_result.message());
                }
            }
        }

        if (auto const post_result = semaphore.post()) {
            logger->error("Failed to post on semaphore {}: {}", 
                semaphore_name, post_result.message());
            return post_result.value();
        }

        logger->info("Posted on semaphore {} to signal producer", semaphore_name);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return EXIT_SUCCESS;
}
