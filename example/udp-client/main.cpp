#include <udp_socket.h>

#include "../utility.hpp"

#include <array>
#include <span>
#include <iostream>

auto main(int argc, char **argv)->int {
    const std::string server_address_option{"-a,--address"};
    const std::string server_port_option{"-p,--port"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {}, "Test UDP client");

    const std::string server_address = app->get_option(server_address_option)->as<std::string>();
    const uint16_t server_port = app->get_option(server_port_option)->as<uint16_t>();

    descriptor::udp_socket client{};

    std::array<uint8_t, 4096> buffer{};

    std::string response{};

    while (response != "bye") {
        fmt::print(fg(fmt::color::light_blue), "REQUEST: ");
        
        std::string request{};
        std::cin >> request;

        {
        auto [result, count] =
            client.write_to(request.cbegin(), request.cend(), server_address, server_port);

        if (result != 0) {
            logger->error("Failed to send to {}:{} with error {} '{}'", 
                server_address, server_port, result, descriptor::descriptor::error_description(result));
            break;
        }
        }
        {
        std::string receive_address{};
        uint16_t receive_port{};

        auto [result, count] =
            client.read_from(buffer,receive_address,receive_port);

        if (result != 0) {
            logger->error("Failed to receive from {}:{} with error {} '{}'", 
                receive_address, receive_port, result, descriptor::descriptor::error_description(result));
            break;
        }
        else {
            response = std::string(static_cast<const char*>(static_cast<const void*>(buffer.data())), count);
            logger->info("Received '{}' ({} bytes) from {}:{}", response, count, receive_address, receive_port);
            fmt::print(fg(fmt::color::light_green), "Received {}\n", response);
        }
        }
    }

    return EXIT_SUCCESS;
}
