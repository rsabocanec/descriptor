#include <message_queue.h>

#include "../utility.hpp"

#include <random>
#include <thread>
#include <chrono>

#include <csignal>

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

auto main(int argc, char **argv)->int {
    using namespace std::chrono_literals;

    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

    const std::string can_device_option{"-m,--message-queue"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {can_device_option}, "Test CAN publisher");

    std::unique_ptr<descriptor::basic_file> publisher {new descriptor::message_queue};

    std::string message_queue_name{};

    try {
        message_queue_name = app->get_option("--message-queue")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    if (auto const result = publisher->open(message_queue_name); result != 0) {
        logger->error("Failed to bind to {}: {}", message_queue_name, descriptor::error_description(result));
        return result;
    }

    descriptor::message_queue::attributes attr{};

    if (auto const result = dynamic_cast<descriptor::message_queue*>(publisher.get())->get_attributes(attr); result != 0) {
        logger->error("Failed to get message queue attributes, with error {} {}", 
            result, descriptor::error_description(result));
        return EXIT_FAILURE;
    }

    std::vector<uint8_t> buffer(attr.max_msg_size_);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<uint8_t> mq_distrib(0, 255);

    while (!stop_flag.load()) {
        const std::size_t len = std::clamp( static_cast<std::size_t>(mq_distrib(gen)), 
                                            static_cast<std::size_t>(0ull), 
                                            static_cast<std::size_t>(attr.max_msg_size_));

        for (std::size_t i = 0; i < len; ++i) {
            buffer.at(i) = mq_distrib(gen);
        }

        auto const [result, count] =
            publisher->write(buffer.cbegin(), len);

        if (result != 0) {            
            logger->error("Failed to send data to message queue: ({}) {}. Sent {} bytes", 
                result, descriptor::error_description(result), count);
            return result;
        }

        if (count != len) {
            logger->error("Failed to send data of length {} to message queue, sent {} instead", 
                len, count - 8);
        }

        std::this_thread::sleep_for(1s);
    }

    fmt::print(fg(fmt::color::green), "\nEXIT!\n");

    return EXIT_SUCCESS;
}
