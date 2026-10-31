#include <udp_socket.h>

#include "../utility.hpp"

#include <array>
#include <iostream>

auto main(int argc, char **argv)->int {
    const std::string server_address_option{"-a,--address"};
    const std::string server_port_option{"-p,--port"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {server_address_option, server_port_option}, "Test UDP client");

    std::string server_address{};
    uint16_t server_port{};

    try {
        server_address = app->get_option("--address")->as<std::string>();
        server_port = app->get_option("--port")->as<uint16_t>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    descriptor::udp_socket client{};

    std::array<uint8_t, 4096> buffer{};

    std::string response{};

    while (response != "bye") {
        fmt::print(fg(fmt::color::light_blue), "REQUEST: ");
        
        std::string request{};
        std::cin >> request;

        if (auto const result = client.write_to(request.cbegin(), request.cend(), server_address, server_port); !result) {
            logger->error("Failed to send to {}:{} with error {} '{}'", 
                server_address, server_port, result.error().value(), result.error().message());
            break;
        }
        
        std::string receive_address{};
        uint16_t receive_port{};

        if (auto const result = client.read_from(buffer, receive_address, receive_port); !result) {
            logger->error("Failed to receive from {}:{} with error {} '{}'", 
                receive_address, receive_port, result.error().value(), result.error().message());
            break;
        }
        else {
            response = std::string(static_cast<const char*>(static_cast<const void*>(buffer.data())), result.value());
            logger->info("Received '{}' ({} bytes) from {}:{}", response, result.value(), receive_address, receive_port);
            fmt::print("Received: ");
            fmt::print(fg(fmt::color::light_green), "{}\n", response);
        }
    }

    return EXIT_SUCCESS;
}
