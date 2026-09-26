#include <message_queue.h>

#include "../utility.hpp"

#include <random>
#include <thread>
#include <chrono>
#include <iomanip>

#include <csignal>
#include <csignal>

#include <endian.h>
#include <byteswap.h>

namespace {
    volatile std::sig_atomic_t signal_status;
    std::atomic_bool stop_flag{false};

}

void signal_handler(int signal) {
    signal_status = signal;

    switch (signal) {
        case SIGTERM:
        case SIGINT:
            stop_flag = true;
            break;
        default:
            break;
    }
}

namespace {
    void incoming_handler(void *ptr) {
        assert(ptr);

        int32_t mq_desc = *(static_cast<int32_t*>(ptr));
        descriptor::message_queue mq(mq_desc);

        descriptor::message_queue::attributes attr{};

        if (auto const result = mq.get_attributes(attr); result != 0) {
            stop_flag = true;
        }

        auto receive_buffer = std::make_unique<uint8_t[]>(attr.max_msg_size_);
        
        auto [result, count] = mq.read(std::as_writable_bytes(std::span(receive_buffer.get(), attr.max_msg_size_)));

        if (result == 0) {
            for (auto i = 0; i < count; ++i) {
                fmt::print("{} ", receive_buffer[i]);
            }

            fmt::println("");
        }
    }
}

auto main(int argc, char **argv)->int {
    using namespace std::chrono_literals;

    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

    const std::string message_queue_option{"-c,--message_queue"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {message_queue_option}, "Test CAN subscriber");

    std::string message_queue_name{};

    try {
        message_queue_name = app->get_option("--can-device")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    std::unique_ptr<descriptor::basic_file> subscriber{new descriptor::message_queue};
    
    if (auto const result = subscriber->open(message_queue_name); result != 0) {
        logger->error("Failed to open message queue {} with result {} {}", 
            message_queue_name, result, descriptor::error_description(result));
        return result;
    }

    if (auto const result = dynamic_cast<descriptor::message_queue*>(subscriber.get())->notify(incoming_handler); result != 0) {
        logger->error("Failed to notify message queue {} with result {} {}",
            message_queue_name, result, descriptor::error_description(result));
        return result;
    }

    while (!stop_flag.load()) {
        std::this_thread::sleep_for(100ms);
    }

    if (auto const result = dynamic_cast<descriptor::message_queue*>(subscriber.get())->notify(); result != 0) {
        logger->error("Failed to clear notify message queue {} with result {} {}",
            message_queue_name, result, descriptor::error_description(result));
    }

    fmt::print(fg(fmt::color::green), "\nEXIT!\n");

    return EXIT_SUCCESS;
}
