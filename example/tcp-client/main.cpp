#include <tcp_socket.h>

#include "../utility.hpp"

#include <array>
#include <span>
#include <iostream>

auto main(int argc, char **argv)->int {

    const std::string server_address_option{"-a,--address"};
    const std::string server_port_option{"-p,--port"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {server_address_option, server_port_option}, "Test TCP client");

    std::string server_address{};

    try {
        server_address = app->get_option("--address")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    uint16_t server_port{};

    try {
        server_port = app->get_option("--port")->as<uint16_t>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    
    descriptor::tcp_socket client{};

    auto result = client.connect(server_address, server_port);
    if (result != 0) {
        logger->error("Failed to connect to {}:{} with error {} '{}'", 
            server_address, server_port, result, descriptor::error_description(result));;
    }
    else {
        std::string request{};

        do {
            fmt::print(fg(fmt::color::light_blue), "\nREQUEST: ");

            std::cin >> request;

            auto const [write_result, bytes_written] =
                client.write(request.cbegin(), request.cend());

            if (write_result != 0) {
                logger->error("Send failed; {} '{}'", write_result, descriptor::error_description(write_result));
            }
            else {
                logger->info("Sent '{}' ({} bytes)\n", request, bytes_written);

                std::array<char, 24> response{};
                auto const [read_result, bytes_read] = client.read(response);
                if (read_result != 0) {
                    logger->error("Receive failed; {} '{}'", read_result, descriptor::error_description(read_result));
                }
                else {
                    logger->info("Received '{}' ({} bytes)\n", std::string_view(response.cbegin(), bytes_read), bytes_read);
                }
            }
        } while (request != "bye");

        fmt::print(fg(fmt::color::green), "\nDisconnecting!\n");

        result = client.disconnect();
        if (result != 0) {
            logger->error("Disconnect failed; {} '{}'", result, descriptor::error_description(result));
        }
    }

    return EXIT_SUCCESS;
}
