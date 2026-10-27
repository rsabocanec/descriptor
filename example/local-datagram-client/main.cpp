#include <local_datagram_socket.h>

#include "../utility.hpp"

#include <array>
#include <memory>
#include <iostream>


auto main(int argc, char **argv)->int {
    const std::string server_filename_option{"-s,--server-file"};
    const std::string client_filename_option{"-c,--client-file"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {server_filename_option, client_filename_option}, "Test local datagram client");

    std::unique_ptr<descriptor::datagram_socket> client =
        std::make_unique<descriptor::local_datagram_socket>();
    
    std::string client_filename{};

    try {
        client_filename = app->get_option("--client-file")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    
    if (auto const result = client->bind(client_filename)) {
        logger->error("Failed to bind to {} with result {} {}", 
            client_filename, result.value(), result.message());
        return result.value();
    }

    std::string server_filename{};

    try {
        server_filename = app->get_option("--server_file")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }
    

    std::array<uint8_t, 4096> buffer{};

    std::string response{};

    while (response != "bye") {
        fmt::print("REQUEST: ");
    
        std::string request{};
        std::cin >> request;

        if (auto const result = client->write_to(request.cbegin(), request.cend(), server_filename); !result) {
            logger->error("Failed to write to {} with result {} {}", 
                server_filename, result.error().value(), result.error().message());
            break;
        }
        
        std::string receive_address{};

        if (auto const result = client->read_from(buffer, receive_address); !result) {
            logger->error("Failed to read from {} with result {} {}", 
                receive_address, result.error().value(), result.error().message());
            break;
        }
        else {
            response = std::string(static_cast<const char*>(static_cast<const void*>(buffer.data())), result.value());
            logger->info("Received {} from {}", response, receive_address);
        }
    }
    
    if (auto const result = descriptor::unlink(client_filename)) {
        logger->error("Failed to unlink {} with result {} {}", 
            client_filename, result.value(), result.message());
    }

    return EXIT_SUCCESS;
}
