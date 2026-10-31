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
        argc, argv, {shmem_filename_option}, "Test shared memory producer");

    std::string shmem_filename;

    try {
        shmem_filename = app->get_option("--shmem-filename")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    descriptor::shared_memory shared_mem{};

    if (auto const open_result = shared_mem.open(shmem_filename, descriptor::open_flags::write_only | descriptor::open_flags::create)) {
        logger->error("Failed to open shared memory {}, reason: {}", 
            shmem_filename, open_result.message());
        return open_result.value();
    }

    logger->info("Shared memory {} opened successfully", shmem_filename);

    descriptor::named_semaphore semaphore{};

    std::string semaphore_name = "/" + std::string(shmem_filename) + "_semaphore";

    if (auto const open_result = semaphore.open(semaphore_name)) {
        logger->error("Failed to open semaphore {}, with error {} '{}'", 
            semaphore_name, open_result.value(), open_result.message());
        return open_result.value();
    }

    logger->info("Semaphore {} opened successfully", semaphore_name);
    
    fmt::print(fg(fmt::color::white), "\nWaiting for a consumer!\n");

    if (auto const wait_result = semaphore.wait()) {
        logger->error("Failed to wait on semaphore {}, with error {} '{}'", 
            semaphore_name, wait_result.value(), wait_result.message());
        return wait_result.value();
    }

    logger->info("Consumer is ready! Starting to write messages to shared memory {}", shmem_filename);

    for (auto i = 0; i < 10; ++i) {
        std::string message = "Message " + std::to_string(i) + " from producer";

        logger->info("Writing message {} to shared memory {}", i, shmem_filename);

        std::string hello_message = "Hello from shared memory producer! Iteration: " + std::to_string(i + 1);

        auto const write_result = shared_mem.write(hello_message.cbegin(), hello_message.cend());
        
        if (!write_result) {
            logger->error("Failed to write to shared memory {}, with error {} '{}'", 
                shmem_filename, write_result.error().value(), write_result.error().message());
        }
        else {
            fmt::print(fg(fmt::color::orange), "{}\n", hello_message);
            logger->info("Wrote {} bytes to shared memory {}", write_result.value(), shmem_filename);
        }

        if (auto const post_result = semaphore.post()) {
            logger->error("Failed to post on semaphore {}, with error {} '{}'", 
                semaphore_name, post_result.value(), post_result.message());
            return post_result.value();
        }

        logger->info("Posted on semaphore {} to signal consumer", semaphore_name);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        logger->info("Waiting on semaphore {} from consumer", semaphore_name);

        if (auto const wait_result = semaphore.wait()) {
            logger->error("Failed to wait on semaphore {}, with error {} '{}'", 
                semaphore_name, wait_result.value(), wait_result.message());
            return wait_result.value();
        }

        logger->info("Consumer has read message {} from shared memory {} and truncated its content", i + 1, shmem_filename);
    }

    return EXIT_SUCCESS;
}
