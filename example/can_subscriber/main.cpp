#include <can_socket.h>

#include <random>
#include <thread>
#include <chrono>
#include <iostream>

#include <csignal>

#include <netinet/in.h>

#include <endian.h>
#include <byteswap.h>

namespace {
    volatile std::sig_atomic_t signal_status;

    rsabocanec::can_socket *g_subscriber{nullptr};
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

auto main()->int {
    using namespace std::chrono_literals;

    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

    rsabocanec::can_socket subscriber{};
    g_subscriber = &subscriber;

    if (auto const result = subscriber.open(); result != 0) {
        std::cerr   << "Failed to open CAN socket: " << result
                    << rsabocanec::descriptor::error_description(result)
                    << std::endl;
        return result;
    }

    constexpr std::string_view address = "vcan0";

    if (auto const result = subscriber.bind(address); result != 0) {
        std::cerr   << "Failed to bind to " << address
                    << " with result " << result
                    << rsabocanec::descriptor::error_description(result)
                    << std::endl;
        return result;
    }

    std::array<uint8_t, rsabocanec::can_frame::can_frame_buffer_size> buffer{};

    for (;;) {
        auto const [result, count] = subscriber.read(buffer);

        if (result != 0) {
            std::cerr   << "Failed to read CAN payload: (" << result << ") "
                        << rsabocanec::descriptor::error_description(result)
                        << '\n';
            return result;
        }

        rsabocanec::can_frame frame(buffer);

        std::cout   << "CAN frame: 0x"
                    << std::hex << std::setfill('0') << std::setw(8)
                    << ::ntohl(frame.header().uint32_id_) << '\t'
                    << std::setw(16)
                    << ::__bswap_64(std::bit_cast<uint64_t>(frame.payload()))
                    << std::endl;
    }

    std::cout << "EXIT!\n";

    return EXIT_SUCCESS;
}
