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

    auto result = client.connect(app->get_option(server_path_option)->as<std::string>());
    if (result != 0) {
        std::cerr << "Connect failed; " << descriptor::descriptor::error_description(result) << '\n';
    }
    else {
        std::string request{};

        do {
            std::cout << "\nREQUEST: ";
            std::cin >> request;

            auto [write_result, bytes_written]  =
                client.write(request.cbegin(), request.cend());

            if (write_result != 0) {
                std::cerr << "Write failed; " << descriptor::descriptor::error_description(write_result) << '\n';
            }
            else {
                std::cout << "Sent '" << request << "'\n";

                std::array<char, 24> response{};
                auto const [read_result, bytes_read] =
                    client.read(response);

                if (read_result != 0) {
                    std::cerr << "Read failed; " << descriptor::descriptor::error_description(read_result) << '\n';
                }
                else {
                    std::cout << "Received '" << std::string_view(response.cbegin(), bytes_read) << "'\n";
                }
            }
        } while (request != "bye");

        result = client.disconnect();
        if (result != 0) {
            std::cerr << "Disconnect failed; " << descriptor::descriptor::error_description(result) << '\n';
        }
    }

    return EXIT_SUCCESS;
}
