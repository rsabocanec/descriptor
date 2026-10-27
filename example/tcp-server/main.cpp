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

                    std::ofstream ofs{"/tmp/received.txt"};

                    descriptor::expected_result read_result{};

                    do {
                        read_result = acceptor.read(request);

                        if (!read_result) {
                            logger->error("Receive failed; {} '{}'", read_result.error().value(), read_result.error().message());
                        }
                        else if (read_result.value() == 0) {
                            logger->info("Connection from {}:{} has been closed!", acceptor.peer(), acceptor.peer_port());
                        }
                        else {
                            const std::string_view response(request.cbegin(), read_result.value());
                            ofs << response;
                            
                            logger->info("Received '{}'", response);
#if 0
                            auto const write_result = acceptor.write(response.cbegin(), response.cend());

                            if (!write_result) {
                                logger->error("Send failed; {} '{}'", 
                                    write_result.error().value(), write_result.error().message());
                            }
                            else {
                                logger->info("Sent '{}'", response);
                            }
#endif                            
                        }
                    } while (read_result.has_value() && read_result.value() > 0);

                    logger->info("Shutting down the acceptor!");

                    [[maybe_unused]] auto shutdown_result = acceptor.shutdown();

                    logger->info("Client {}:{} disconnected!", acceptor.peer(), acceptor.peer_port());

                }, std::move(accept_result.value())).detach();

                accept_result = server.accept();
            }

            logger->error("Accept failed; {} '{}'", 
                accept_result.error().value(), accept_result.error().message());
        }
    }

    fmt::print(fg(fmt::color::green), "\nEXIT!\n");
    return EXIT_SUCCESS;
}
