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

        descriptor::message_queue mq(*mq_desc);

        descriptor::message_queue::attributes attr{};

        if (auto const attr_result = mq.get_attributes(attr); attr_result != 0) {
            stop_flag = true;
            fmt::print(fg(  fmt::color::crimson), "Failed to get attributes, with error {} {}", 
                            attr_result, descriptor::error_description(attr_result));
        }
        else {
            auto receive_buffer = std::make_unique<uint8_t[]>(attr.max_msg_size_);
            
            auto [result, count] = mq.read(std::as_writable_bytes(std::span(receive_buffer.get(), attr.max_msg_size_)));

            fmt::println("Received {} bytes:", count);

            if (result == 0) {
                for (auto i = 0; i < count; ++i) {
                    fmt::print("0x{:02x} ", static_cast<uint16_t>(receive_buffer[i]));
                }

                fmt::println("");

                if (auto const notify_result = mq.notify(incoming_handler); notify_result != 0) {
                    fmt::print( fg(fmt::color::crimson), "Failed to notify message queue {} with result {} {}\n",
                                *mq_desc, notify_result, descriptor::error_description(notify_result));
                    stop_flag = true;
                }
                else {
                    fmt::println("Successfully set notification handler!");
                }
            }
            else {
                fmt::print( fg(fmt::color::crimson), "Failed to read from the message queue, with error {} {}\n", 
                            result, descriptor::error_description(result));

                stop_flag = true;
            }
        }

        mq.release();
        
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
