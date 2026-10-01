#include <local_stream_socket.h>

#include "../utility.hpp"

#include <array>
#include <span>

#include <iostream>

auto main(int argc, char **argv)->int {
    const std::string server_path_option{"-s,--server-path"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {server_path_option}, "Test local stream server");

    descriptor::local_stream_socket client{};

    std::string server_path{};

    try {
        server_path = app->get_option("--server-path")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    auto result = client.connect(server_path);
    if (result != 0) {
        std::cerr << "Connect failed; " << descriptor::error_description(result) << '\n';
    }
    else {
        std::string request{};

        do {
            std::cout << "\nREQUEST: ";
            std::cin >> request;

            auto [write_result, bytes_written]  =
                client.write(request.cbegin(), request.cend());

            if (write_result != 0) {
                std::cerr << "Write failed; " << descriptor::error_description(write_result) << '\n';
            }
            else {
                std::cout << "Sent '" << request << "'\n";

                std::array<char, 24> response{};
                auto const [read_result, bytes_read] =
                    client.read(response);

                if (read_result != 0) {
                    std::cerr << "Read failed; " << descriptor::error_description(read_result) << '\n';
                }
                else {
                    std::cout << "Received '" << std::string_view(response.cbegin(), bytes_read) << "'\n";
                }
            }
        } while (request != "bye");

        result = client.disconnect();
        if (result != 0) {
            std::cerr << "Disconnect failed; " << descriptor::error_description(result) << '\n';
        }
    }

    return EXIT_SUCCESS;
}
