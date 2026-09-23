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

    const std::string shmem_filename = app->get_option(shmem_filename_option)->as<std::string>();

    descriptor::shared_memory shared_mem{};

    if (auto const open_result = shared_mem.open(shmem_filename, descriptor::open_flags::write_only | descriptor::open_flags::create); open_result != 0) {
        logger->error("Failed to open shared memory {}: {}", 
            shmem_filename, descriptor::error_description(open_result));
        return open_result;
    }

    logger->info("Shared memory {} opened successfully", shmem_filename);

    descriptor::named_semaphore semaphore{};

    std::string semaphore_name = "/" + std::string(shmem_filename) + "_semaphore";

    if (auto const open_result = semaphore.open(semaphore_name); open_result != 0) {
        logger->error("Failed to open semaphore {}, with error {} '{}'", 
            semaphore_name, open_result, descriptor::error_description(open_result));
        return open_result;
    }

    logger->info("Semaphore {} opened successfully", semaphore_name);
    
    fmt::print(fg(fmt::color::white), "\nWaiting for a consumer!\n");

    if (auto const wait_result = semaphore.wait(); wait_result != 0) {
        logger->error("Failed to wait on semaphore {}, with error {} '{}'", 
            semaphore_name, wait_result, descriptor::error_description(wait_result));
        return wait_result;
    }

    logger->info("Consumer is ready! Starting to write messages to shared memory {}", shmem_filename);

    for (auto i = 0; i < 10; ++i) {
        std::string message = "Message " + std::to_string(i) + " from producer";

        logger->info("Writing message {} to shared memory {}", i, shmem_filename);

        std::string hello_message = "Hello from shared memory producer! Iteration: " + std::to_string(i + 1);

        auto const [write_result, bytes_written] = shared_mem.write(hello_message.cbegin(), hello_message.cend());
        
        if (write_result != 0) {
            logger->error("Failed to write to shared memory {}, with error {} '{}'", 
                shmem_filename, write_result, descriptor::error_description(write_result));
        }
        else {
            fmt::print(fg(fmt::color::orange), "{}\n", hello_message);
            logger->info("Wrote {} bytes to shared memory {}", bytes_written, shmem_filename);
        }

        if (auto const post_result = semaphore.post(); post_result != 0) {
            logger->error("Failed to post on semaphore {}, with error {} '{}'", 
                semaphore_name, post_result, descriptor::error_description(post_result));
            return post_result;
        }

        logger->info("Posted on semaphore {} to signal consumer", semaphore_name);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        logger->info("Waiting on semaphore {} from consumer", semaphore_name);

        if (auto const wait_result = semaphore.wait(); wait_result != 0) {
            logger->error("Failed to wait on semaphore {}, with error {} '{}'", 
                semaphore_name, wait_result, descriptor::error_description(wait_result));
            return wait_result;
        }

        logger->info("Consumer has read message {} from shared memory {} and truncated its content", i + 1, shmem_filename);
    }

    return EXIT_SUCCESS;
}
