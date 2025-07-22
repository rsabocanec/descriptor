#include <udp_socket.h>

#include <array>
#include <span>

#include <iostream>

auto main()->int {
    rsabocanec::udp_socket server{};

    if (auto const result = server.open(); result != 0) {
        std::cerr   << "Failed to open UDP socket: " << result
                    << rsabocanec::descriptor::error_description(result)
                    << std::endl;
        return result;
    }

    constexpr std::string_view address = "127.0.0.1";
    constexpr uint16_t port = 9999;

    if (auto const result = server.bind(address, port); result != 0) {
        std::cerr   << "Failed to bind to " << address << ':' << port
                    << " with result " << result
                    << rsabocanec::descriptor::error_description(result)
                    << std::endl;
        return result;
    }

    std::array<uint8_t, 4096> buffer{};

    std::string request{};

    while (request != "bye") {
        std::string receive_address{};
        uint16_t receive_port{};

        {
        auto [result, count] = server.read_from(
            std::as_writable_bytes(std::span<uint8_t>(buffer)),
            receive_address,
            receive_port);

        if (result != 0) {
            std::cerr   << "Failed to read with result "
                        << result << ' '
                        << rsabocanec::descriptor::error_description(result)
                        << std::endl;
            break;
        }
        else {
            request = std::string(static_cast<const char*>(static_cast<const void*>(buffer.data())), count);
            std::cout << "Received " << request << '\n';
        }
        }
        {
        auto [result, count] = server.write_to(
            std::as_bytes(std::span<const char>(request.cbegin(), request.cend())),
            receive_address,
            receive_port);

        if (result != 0) {
            std::cerr   << "Failed to write to "
                        << receive_address << ':' << receive_port
                        << " with result " << result << ' '
                        << rsabocanec::descriptor::error_description(result)
                        << std::endl;
            break;
        }
        }
    }
    return EXIT_SUCCESS;
}
