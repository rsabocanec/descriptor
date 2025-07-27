#include <can_socket.h>

#include <random>
#include <thread>
#include <chrono>
#include <iostream>

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

auto main()->int {
    using namespace std::chrono_literals;

    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

    rsabocanec::can_socket publisher{};

    if (auto const result = publisher.open(); result != 0) {
        std::cerr   << "Failed to open CAN socket: " << result
                    << rsabocanec::descriptor::error_description(result)
                    << std::endl;
        return result;
    }

    constexpr std::string_view address = "vcan0";

    if (auto const result = publisher.bind(address); result != 0) {
        std::cerr   << "Failed to bind to " << address
                    << " with result " << result
                    << rsabocanec::descriptor::error_description(result)
                    << std::endl;
        return result;
    }

    std::vector<uint8_t> buffer(rsabocanec::can_socket_frame::can_frame_buffer_size);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<uint32_t> can_id_distrib{};
    std::uniform_int_distribution<uint64_t> can_payload_distrib{};
    std::uniform_int_distribution<uint8_t> can_payload_length_distrib(1, 8);

    rsabocanec::can_socket_frame frame{};

    while (!stop_flag.load()) {
        frame.header(can_id_distrib(gen));

        auto const pl = can_payload_distrib(gen);
        frame.payload(std::bit_cast<rsabocanec::can_payload>(pl));

        frame.payload_length(can_payload_length_distrib(gen));

        auto const [ser_result, ser_count] = frame.as_bytes(buffer.begin(), buffer.end());

        if (ser_result != 0) {
            std::cerr   << "Failed to serialize CAN frame: "
                        << ser_result << ", count: " << ser_count << '\n';
            return ser_result;
        }

        auto const [result, count] =
            publisher.write(buffer.cbegin(), buffer.cbegin() + ser_count);

        if (result != 0) {
            std::cerr   << "Failed to send CAN payload: (" << result << ") "
                        << rsabocanec::descriptor::error_description(result)
                        << ". Sent " << count << " bytes\n";
            return result;
        }

        if (count != ser_count) {
            std::cout   << "Failed to send CAN payload length ("
                        << static_cast<uint16_t>(frame.payload_length())
                        << "), sent " << count - 8
                        << " instead\n";
        }

        std::this_thread::sleep_for(100ms);
    }

    std::cout << "EXIT!\n";

    return EXIT_SUCCESS;
}
