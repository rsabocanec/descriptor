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

    try {
        filename = app->get_option("--filename")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}", onf.what());
        return EXIT_FAILURE;
    }

    std::string address{};

    try {
        address = app->get_option("--address")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}", onf.what());
        return EXIT_FAILURE;
    }

    uint16_t port{};

    try {
        port = app->get_option("--port")->as<uint16_t>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}", onf.what());
        return EXIT_FAILURE;
    }

    descriptor::file file{};

    auto result = file.open(filename, descriptor::open_flags::read_only);
    if (result != 0) {
        logger->error("Failed to open file {} with result {} '{}'", 
            filename, result, descriptor::error_description(result));
        return result;
    }

    descriptor::tcp_socket client{};

    result = client.connect(address, port);
    if (result != 0) {
        logger->error("Connect to {}:{} failed; {}", address, port, descriptor::error_description(result));
    }
    else {
        auto const [splice_result, bytes_spliced] = client.splice(file, file.size());

        if (splice_result != 0) {
            logger->error("Splice failed; {}", descriptor::error_description(splice_result));
        }
        else {
            logger->info("Spliced {} bytes from file {} to TCP socket {}:{}", bytes_spliced, filename, address, port);
        }

        fmt::print(fg(fmt::color::green), "\nDisconnecting!\n");

        result = client.disconnect();
        if (result != 0) {
            logger->error("Disconnect failed; {}", descriptor::error_description(result));
        }
    }

    result = file.close();
    if (result != 0) {
        logger->error("Failed to close file {} with error {} '{}'", 
            filename, result, descriptor::error_description(result));
    }

    fmt::print(fg(fmt::color::green), "\nEXIT!\n");

    return EXIT_SUCCESS;
}
