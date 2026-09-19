#include <can_socket.h>

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

    const std::string can_device_option{"-c,--can-device"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {can_device_option}, "Test CAN publisher");

    descriptor::can_socket publisher{};

    const std::string can_device = app->get_option(can_device_option)->as<std::string>();

    if (auto const result = publisher.bind(can_device); result != 0) {
        logger->error("Failed to bind to {}: {}", can_device, descriptor::descriptor::error_description(result));
        return result;
    }

    std::vector<uint8_t> buffer(descriptor::can_socket_frame::can_frame_buffer_size);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<uint32_t> can_id_distrib{};
    std::uniform_int_distribution<uint64_t> can_payload_distrib{};
    std::uniform_int_distribution<uint8_t> can_payload_length_distrib(1, 8);

    descriptor::can_socket_frame frame{};

    while (!stop_flag.load()) {
        frame.header(can_id_distrib(gen));

        auto const pl = can_payload_distrib(gen);
        frame.payload(std::bit_cast<descriptor::can_payload>(pl));

        frame.payload_length(can_payload_length_distrib(gen));

        auto const [ser_result, ser_count] = frame.as_bytes(buffer.begin(), buffer.end());

        if (ser_result != 0) {
            logger->error("Failed to serialize CAN frame: {}, count: {}", ser_result, ser_count);
            return ser_result;
        }

        auto const [result, count] =
            publisher.write(buffer.cbegin(), buffer.cbegin() + ser_count);

        if (result != 0) {            
            logger->error("Failed to send CAN payload: ({}) {}. Sent {} bytes", 
                result, descriptor::descriptor::error_description(result), count);
            return result;
        }

        if (count != ser_count) {
            logger->error("Failed to send CAN payload length ({}), sent {} instead", 
                frame.payload_length(), count - 8);
        }

        std::this_thread::sleep_for(100ms);
    }

    fmt::print(fg(fmt::color::green), "\nEXIT!\n");

    return EXIT_SUCCESS;
}
