#include <local_datagram_socket.h>

#include "../utility.hpp"

#include <array>
#include <memory>

auto main(int argc, char **argv)->int {

    const std::string socket_filename_option{"-s,--socket-file"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {socket_filename_option}, "Test poll() function");
        
    std::string socket_filename{};

    try {
        socket_filename = app->get_option("--socket-file")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}", onf.what());
        return EXIT_FAILURE;
    }

    std::unique_ptr<descriptor::datagram_socket> server =
        std::make_unique<descriptor::local_datagram_socket>();

    if (auto const result = server->bind(socket_filename); result != 0) {
        logger->error("Failed to bind to {} with result {} {}", 
            socket_filename, result, descriptor::error_description(result));
        return result;
    }

    std::array<uint8_t, 4096> buffer{};

    std::string request{};

    while (request != "bye") {
        std::string receive_address{};

        {
        auto [result, count] =
            server->read_from(buffer, receive_address);

        if (result != 0) {
            logger->error("Failed to read from {} with result {} {}", 
                receive_address, result, descriptor::error_description(result));
            break;
        }
        else {
            request = std::string(static_cast<const char*>(static_cast<const void*>(buffer.data())), count);
            logger->info("Received {} from {}", request, receive_address);
        }
        }
        {
        auto [result, count] =
            server->write_to(request.cbegin(), request.cend(), receive_address);

        if (result != 0) {
            logger->error("Failed to write to {} with result {} {}", 
                receive_address, result, descriptor::error_description(result));
            break;
        }
        }
    }

    if (auto const result = descriptor::unlink(socket_filename); result != 0) {
        logger->error("Failed to unlink {} with result {} {}", 
            socket_filename, result, descriptor::error_description(result));
    }
    
    return EXIT_SUCCESS;
}
