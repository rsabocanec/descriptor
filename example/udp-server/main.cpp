#include <udp_socket.h>

#include "../utility.hpp"

#include <array>
#include <span>

auto main(int argc, char **argv)->int {

    const std::string server_address_option{"-a,--address"};
    const std::string server_port_option{"-p,--port"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {server_address_option, server_port_option}, "Test UDP server");

    descriptor::udp_socket server{};

    const std::string server_address = app->get_option(server_address_option)->as<std::string>();
    const uint16_t server_port = app->get_option(server_port_option)->as<uint16_t>();

    if (auto const result = server.bind(server_address, server_port); result != 0) {
        logger->error("Failed to bind to {}:{} with result {} '{}'", 
            server_address, server_port, result, descriptor::descriptor::error_description(result));
        return result;
    }

    std::array<uint8_t, 4096> buffer{};

    std::string request{};

    while (request != "bye") {
        std::string receive_address{};
        uint16_t receive_port{};

        {
        auto [result, count] =
            server.read_from(buffer, receive_address, receive_port);

        if (result != 0) {
            logger->error("Failed to receive from {}:{} with error {} '{}'", 
                receive_address, receive_port, result, descriptor::descriptor::error_description(result));
            break;
        }
        else {
            request = std::string(static_cast<const char*>(static_cast<const void*>(buffer.data())), count);
            logger->info("Received {} from {}:{}", request, receive_address, receive_port);
            fmt::print(fg(fmt::color::light_green), "Received {}\n", request);
        }
        }
        {
        auto [result, count] =
            server.write_to(request.cbegin(), request.cend(),receive_address, receive_port);

        if (result != 0) {
            logger->error("Failed to send to {}:{} with error {} '{}'", 
                receive_address, receive_port, result, descriptor::descriptor::error_description(result));
            break;
        }
        }
    }
    return EXIT_SUCCESS;
}
