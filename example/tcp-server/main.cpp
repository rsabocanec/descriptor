#include <tcp_socket.h>

#include <array>
#include <thread>
#include <iostream>

#include <csignal>

namespace {
    volatile std::sig_atomic_t signal_status;

    rsabocanec::tcp_socket *g_server{nullptr};
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

auto main()->int {
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

    rsabocanec::tcp_socket server{};
    g_server = &server;

    auto result = server.bind("127.0.0.1", 9999);
    if (result != 0) {
        std::cerr << "bind failed; " << rsabocanec::descriptor::error_description(result) << '\n';
    }
    else {
        result = server.listen();
        if (result != 0) {
            std::cerr << "listen faeled: " << rsabocanec::descriptor::error_description(result) << '\n';
        }
        else {
            auto accept_result = server.accept();
            while (accept_result.has_value()) {
                std::thread([](rsabocanec::acceptor&& acceptor) {
                    std::cout << "New connection from " << acceptor.peer() << ':' << acceptor.peer_port() << '\n';

                    std::array<char, 24> request{};

                    int32_t read_error{};
                    int32_t bytes_read{};

                    do {
                        auto const read_result = acceptor.read(request);
                        read_error = std::get<0>(read_result);
                        bytes_read = std::get<1>(read_result);

                        if (read_error != 0) {
                            std::cerr << "read failed; " << rsabocanec::descriptor::error_description(read_error) << '\n';
                        }
                        else if (bytes_read == 0) {
                            std::cout << "Connection from " << acceptor.peer() << ':' << acceptor.peer_port()
                                      << " has been closed!\n";
                        }
                        else {
                            const std::string_view response(request.cbegin(), bytes_read);
                            std::cout << "Received '" << response << "'\n";

                            auto const [write_result, bytes_written] =
                                acceptor.write(response.cbegin(), response.cend());

                            if (write_result != 0) {
                                std::cerr << "Write failed; " << rsabocanec::descriptor::error_description(write_result) << '\n';
                            }
                            else {
                                std::cout << "Sent '" << response << "'\n";
                            }
                        }
                    } while (read_error == 0 && bytes_read > 0);

                    std::cout << "Shutting down the acceptor!\n";

                    [[maybe_unused]] auto shutdown_result = acceptor.shutdown();

                    std::cout << "Client " <<  acceptor.peer() << ':' << acceptor.peer_port()
                              << " disconnected!\n";

                }, std::move(accept_result.value())).detach();

                accept_result = server.accept();
            }

            std::cerr << "Accept failed: " << rsabocanec::descriptor::error_description(accept_result.error()) << '\n';
        }
    }

    std::cout << "EXIT\n";
    return EXIT_SUCCESS;
}
