#include <udp_socket.h>

#include <array>
#include <span>

#include <iostream>

auto main()->int {
    rsabocanec::udp_socket client{};

    if (auto const result = client.open(); result != 0) {
        std::cerr   << "Failed to open UDP socket: " << result
                    << rsabocanec::descriptor::error_description(result)
                    << std::endl;
        return result;
    }

    constexpr std::string_view address = "127.0.0.1";
    constexpr uint16_t port = 9999;

    std::array<uint8_t, 4096> buffer{};

    std::string response{};

    while (response != "bye") {
        std::cout << "REQUEST: ";
        std::string request{};
        std::cin >> request;

        {
        auto [result, count] =
            client.write_to(request.cbegin(), request.cend(), address, port);

        if (result != 0) {
            std::cerr   << "Failed to write to "
                        << address << ':' << port
                        << " with result " << result << ' '
                        << rsabocanec::descriptor::error_description(result)
                        << std::endl;
            break;
        }
        }
        {
        std::string receive_address{};
        uint16_t receive_port{};

        auto [result, count] =
            client.read_from(buffer,receive_address,receive_port);

        if (result != 0) {
            std::cerr   << "Failed to read with result "
                        << result << ' '
                        << rsabocanec::descriptor::error_description(result)
                        << std::endl;
            break;
        }
        else {
            response = std::string(static_cast<const char*>(static_cast<const void*>(buffer.data())), count);
            std::cout << "Received " << response << '\n';
        }
        }
    }

    return EXIT_SUCCESS;
}
