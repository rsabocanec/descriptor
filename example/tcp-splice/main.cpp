#include <tcp_socket.h>
#include <file.h>

#include "../utility.hpp"

#include <array>
#include <span>


auto main(int argc, char** argv)->int {

    const std::string filename_option{"-f,--filename"};
    const std::string address_option{"-a,--address"};
    const std::string port_option{"-p,--port"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {filename_option, address_option, port_option}, "Test TCP splice");


    std::string filename{};
    std::string address{};
    uint16_t port{};

    try {
        filename = app->get_option("--filename")->as<std::string>();
        address = app->get_option("--address")->as<std::string>();
        port = app->get_option("--port")->as<uint16_t>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    descriptor::file file{};

    auto result = file.open(filename, descriptor::open_flags::read_only);
    if (result) {
        logger->error("Failed to open file {} with result {} '{}'", 
            filename, result.value(), result.message());
        return result.value();
    }

    descriptor::tcp_socket client{};

    result = client.connect(address, port);
    if (result) {
        logger->error("Connect to {}:{} failed; {}", address, port, result.message());
    }
    else {
        auto const splice_result = client.splice(file, file.size());

        if (!splice_result) {
            logger->error("Splice failed; {}", splice_result.error().message());
        }
        else {
            logger->info("Spliced {} bytes from file {} to TCP socket {}:{}", splice_result.value(), filename, address, port);
        }

        fmt::print(fg(fmt::color::green), "\nDisconnecting!\n");

        result = client.disconnect();
        if (result) {
            logger->error("Disconnect failed; {}", result.message());
        }
    }

    result = file.close();
    if (result) {
        logger->error("Failed to close file {} with error {} '{}'", 
            filename, result.value(), result.message());
    }

    fmt::print(fg(fmt::color::green), "\nEXIT!\n");

    return EXIT_SUCCESS;
}
