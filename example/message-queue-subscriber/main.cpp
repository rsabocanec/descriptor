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
    void incoming_handler(std::unique_ptr<int32_t> mq_desc) {
        assert(mq_desc);

        fmt::println("Handling notification from descriptor {}", *mq_desc);

        descriptor::weak_descriptor<descriptor::message_queue> weak_mq{*mq_desc};

        descriptor::message_queue::attributes attr{};

        if (auto const attr_result = weak_mq.load().get_attributes(attr)) {
            stop_flag = true;
            fmt::print(fg(  fmt::color::crimson), "Failed to get attributes, with error {} {}",
                            attr_result.value(), attr_result.message());
        }
        else {
            auto receive_buffer = std::make_unique<uint8_t[]>(attr.max_msg_size_);

            auto const result = weak_mq.load().read(
                std::as_writable_bytes(std::span(receive_buffer.get(), attr.max_msg_size_)));

            if (result.has_value()) {
                fmt::println("Received {} bytes:", result.value());

                for (auto i = 0; i < result.value(); ++i) {
                    fmt::print("0x{:02x} ", static_cast<uint16_t>(receive_buffer[i]));
                }

                fmt::println("");
            }
            else {
                fmt::print( fg(fmt::color::crimson), "Failed to read from the message queue, with error {} {}\n",
                            result.error().value(), result.error().message());

                stop_flag = true;
            }
        }

        if (auto const attr_result = weak_mq.load().get_attributes(attr)) {
            stop_flag = true;
            fmt::print(fg(  fmt::color::crimson), "Failed to get attributes, with error {} {}",
                            attr_result.value(), attr_result.message());
        }
        else {
            fmt::println("Messages left in the queue: {}", attr.current_msg_count_);
        }

        if (auto const notify_result = weak_mq.load().notify(incoming_handler)) {
            fmt::print( fg(fmt::color::crimson), "Failed to notify message queue {} with result {} {}\n",
                        *mq_desc, notify_result.value(), notify_result.message());
            stop_flag = true;
        }
        else {
            fmt::println("Successfully set notification handler!");
        }

        fmt::println("Exiting notification handler");
    }
}

auto main(int argc, char **argv)->int {
    using namespace std::chrono_literals;

    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

    const std::string message_queue_option{"-m,--message_queue"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {message_queue_option}, "Test CAN subscriber");

    std::string message_queue_name{};

    try {
        message_queue_name = app->get_option("--message_queue")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    std::unique_ptr<descriptor::basic_file> subscriber{new descriptor::message_queue};

    if (auto const result = subscriber->open(message_queue_name)) {
        logger->error("Failed to open message queue {} with result {} {}",
            message_queue_name, result.value(), result.message());
        return result.value();
    }

    if (auto const result = dynamic_cast<descriptor::message_queue*>(subscriber.get())->notify(incoming_handler)) {
        logger->error("Failed to notify message queue {} with result {} {}",
            message_queue_name, result.value(), result.message());
        return result.value();
    }

    while (!stop_flag.load()) {
        std::this_thread::sleep_for(100ms);
    }

    std::this_thread::sleep_for(2s);

    if (auto const result = subscriber->close()) {
        logger->error("Failed to close message queue {} with result {} {}",
            message_queue_name, result.value(), result.message());
    }

    fmt::print(fg(fmt::color::green), "\nEXIT!\n");

    return EXIT_SUCCESS;
}
