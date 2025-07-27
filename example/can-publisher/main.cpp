#include <can_socket.h>

#include <random>
#include <thread>
#include <chrono>
#include <iostream>

auto main()->int {
    using namespace std::chrono_literals;

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

    std::array<uint8_t, rsabocanec::can_frame::can_frame_buffer_size> buffer{};

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<uint32_t> can_id_distrib{};
    std::uniform_int_distribution<uint64_t> can_payload_distrib{};
    std::uniform_int_distribution<uint8_t> can_payload_length_distrib(1, 8);

    rsabocanec::can_frame frame{};

    for (;;) {
        frame.header(can_id_distrib(gen));

        auto const pl = can_payload_distrib(gen);
        frame.payload(std::bit_cast<rsabocanec::can_payload>(pl));

        frame.payload_length(can_payload_length_distrib(gen));
        {
        auto const [result, count] = frame.as_bytes(buffer);

        if (result != 0) {
            std::cerr   << "Failed to serialize CAN frame: "
                        << result << '\n';
            return result;
        }
        }
        {
        auto const [result, count] = publisher.write(buffer);

        if (result != 0) {
            std::cerr   << "Failed to send CAN payload: (" << result << ") "
                        << rsabocanec::descriptor::error_description(result)
                        << '\n';
            return result;
        }

        if (count != sizeof(rsabocanec::can_header) + frame.payload_length()) {
            std::cout   << "Failed to send CAN payload length ("
                        << frame.payload_length() << "), "
                        << "sent " << count - sizeof(rsabocanec::can_header)
                        << " instead\n";
        }
        }

        std::this_thread::sleep_for(100ms);
    }

    return EXIT_SUCCESS;
}
