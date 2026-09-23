#include <tcp_socket.h>

#include "../utility.hpp"

#include <array>
#include <thread>
#include <fstream>

#include <csignal>

namespace {
    volatile std::sig_atomic_t signal_status;

    descriptor::tcp_socket *g_server{nullptr};
}

void signal_handler(int signal) {
    signal_status = signal;

    switch (signal) {
        case SIGTERM:
            if (g_server != nullptr) {
                [[maybe_unused]] auto const result = g_server->shutdown();
                g_server = nullptr;
            }
            break;
        default:
            break;
    }
}

auto main(int argc, char **argv)->int {
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

    const std::string server_address_option{"-a,--address"};
    const std::string server_port_option{"-p,--port"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {server_address_option, server_port_option}, "Test TCP server");
    
    const std::string server_address = app->get_option(server_address_option)->as<std::string>();
    const uint16_t server_port = app->get_option(server_port_option)->as<uint16_t>();

    descriptor::tcp_socket server{};
    g_server = &server;

    auto result = server.bind(server_address, server_port);
    if (result != 0) {
        logger->error("Bind failed; {} '{}'", result, descriptor::error_description(result));
    }
    else {
        logger->info("Server bound to {}:{}", server_address, server_port);

        result = server.listen();
        if (result != 0) {
            logger->error("Listen failed; {} '{}'", result, descriptor::error_description(result));
        }
        else {
            logger->info("Server listening on {}:{}", server_address, server_port);
            
            auto accept_result = server.accept();
            while (accept_result.has_value()) {
                std::thread([logger](descriptor::acceptor&& acceptor) {
                    logger->info("New connection from {}:{}", acceptor.peer(), acceptor.peer_port());

                    std::array<char, 24> request{};

                    int32_t read_error{};
                    int32_t bytes_read{};

                    std::ofstream ofs{"/tmp/received.txt"};

                    do {
                        auto const read_result = acceptor.read(request);
                        read_error = std::get<0>(read_result);
                        bytes_read = std::get<1>(read_result);

                        if (read_error != 0) {
                            logger->error("Receive failed; {} '{}'", read_error, descriptor::error_description(read_error));
                        }
                        else if (bytes_read == 0) {
                            logger->info("Connection from {}:{} has been closed!", acceptor.peer(), acceptor.peer_port());
                        }
                        else {
                            const std::string_view response(request.cbegin(), bytes_read);
                            ofs << response;
                            
                            logger->info("Received '{}'", response);
#if 0
                            auto const [write_result, bytes_written] =
                                acceptor.write(response.cbegin(), response.cend());

                            if (write_result != 0) {
                                logger->error("Send failed; {} '{}'", write_result, descriptor::error_description(write_result));
                            }
                            else {
                                logger->info("Sent '{}'", response);
                            }
#endif                            
                        }
                    } while (read_error == 0 && bytes_read > 0);

                    logger->info("Shutting down the acceptor!");

                    [[maybe_unused]] auto shutdown_result = acceptor.shutdown();

                    logger->info("Client {}:{} disconnected!", acceptor.peer(), acceptor.peer_port());

                }, std::move(accept_result.value())).detach();

                accept_result = server.accept();
            }

            logger->error("Accept failed; {} '{}'", accept_result.error(), descriptor::error_description(accept_result.error()));
        }
    }

    fmt::print(fg(fmt::color::green), "\nEXIT!\n");
    return EXIT_SUCCESS;
}
