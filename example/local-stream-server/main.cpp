#include <local_stream_socket.h>

#include "../utility.hpp"

#include <array>
#include <thread>
#include <iostream>

#include <csignal>

namespace {
    volatile std::sig_atomic_t signal_status;

    descriptor::local_stream_socket *g_server{nullptr};

    std::string server_path{};
}

void signal_handler(int signal) {
    signal_status = signal;

    switch (signal) {
        case SIGTERM:
	case SIGINT:
            if (g_server != nullptr) {
                [[maybe_unused]] auto const result = g_server->shutdown();
                g_server = nullptr;
            }
	        ::unlink(server_path.c_str());
            break;
        default:
            break;
    }
}

auto main(int argc, char **argv)->int {
    std::signal(SIGTERM, signal_handler);

    const std::string server_path_option{"-s,--server-path"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {server_path_option}, "Test local stream server");

    try {
        server_path = app->get_option("--server-path")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}", onf.what());
        return EXIT_FAILURE;
    }

    descriptor::local_stream_socket server{};
    g_server = &server;

    auto result = server.bind(server_path);
    if (result != 0) {
        std::cerr << "bind failed; " << descriptor::error_description(result) << '\n';
    }
    else {
        result = server.listen();
        if (result != 0) {
            std::cerr << "listen faeled: " << descriptor::error_description(result) << '\n';
        }
        else {
            auto accept_result = server.accept();
            while (accept_result.has_value()) {
                std::thread([](descriptor::acceptor&& acceptor) {
                    std::cout << "New connection accepted\n";

                    std::array<char, 24> request{};

                    int32_t read_error{};
                    int32_t bytes_read{};

                    do {
                        auto const read_result = acceptor.read(request);

                        read_error = std::get<0>(read_result);
                        bytes_read = std::get<1>(read_result);

                        if (read_error != 0) {
                            std::cerr << "read failed; " << descriptor::error_description(read_error) << '\n';
                        }
                        else if (bytes_read == 0) {
                            std::cout << "Connection from " << acceptor.peer() << " has been closed!\n";
                        }
                        else {
                            const std::string_view response(request.cbegin(), bytes_read);
                            std::cout << "Received '" << response << "'\n";

                            auto const [write_result, bytes_written] =
                                acceptor.write(response.cbegin(), response.cend());

                            if (write_result != 0) {
                                std::cerr << "write failed; " << descriptor::error_description(write_result) << '\n';
                            }
                            else {
                                std::cout << "Sent '" << response << "'\n";
                            }
                        }
                    } while (read_error == 0 && bytes_read > 0);

                    [[maybe_unused]] auto shutdown_result = acceptor.shutdown();
                }, std::move(accept_result.value())).detach();

                accept_result = server.accept();
            }

            std::cerr << "accept failed: " << descriptor::error_description(accept_result.error()) << '\n';
        }
    }

    std::cout << "EXIT\n";

    ::unlink(server_path.data());

    return EXIT_SUCCESS;
}
