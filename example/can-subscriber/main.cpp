#include <can_socket.h>

#include "../utility.hpp"

#include <random>
#include <thread>
#include <chrono>
#include <iomanip>

#include <csignal>

#include <netinet/in.h>

#include <endian.h>
#include <byteswap.h>

namespace {
    volatile std::sig_atomic_t signal_status;

    descriptor::can_socket *g_subscriber{nullptr};
}

void signal_handler(int signal) {
    signal_status = signal;

    switch (signal) {
        case SIGTERM:
        case SIGINT:
            if (g_subscriber != nullptr) {
                [[maybe_unused]] auto const result = g_subscriber->shutdown();
                g_subscriber = nullptr;
            }
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
        argc, argv, {can_device_option}, "Test CAN subscriber");

    const std::string can_device = app->get_option(can_device_option)->as<std::string>();

    descriptor::can_socket subscriber{};
    g_subscriber = &subscriber;
    
    if (auto const result = subscriber.bind(can_device); result != 0) {
        logger->error("Failed to bind to {} with result {} {}", 
            can_device, result, descriptor::descriptor::error_description(result));
        return result;
    }

    std::vector<uint8_t> buffer(descriptor::can_socket_frame::can_frame_buffer_size);

    for (;;) {
        auto const [result, count] = subscriber.read(buffer.begin(), buffer.end());

        if (result != 0) {
            logger->error("Failed to read CAN payload: ({}) {}", 
                result, descriptor::descriptor::error_description(result));
            return result;
        }

        descriptor::can_socket_frame frame(buffer.cbegin(), buffer.cend());

        logger->info("CAN frame: 0x{:08x}\t{:016x}", 
            ::ntohl(frame.header().uint32_id_), ::__bswap_64(std::bit_cast<uint64_t>(frame.payload())));
    }

    fmt::print(fg(fmt::color::green), "\nEXIT!\n");

    return EXIT_SUCCESS;
}
