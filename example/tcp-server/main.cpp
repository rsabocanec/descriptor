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

    rsabocanec::tcp_socket server{};
    g_server = &server;

    auto result = server.bind("127.0.0.1", 9999);
    if (result != 0) {
        std::cerr << "bind failed; " << rsabocanec::descriptor::error_description(result) << '\n';
    }
    else {
        result = server.listen(1);
        if (result != 0) {
            std::cerr << "listen faeled: " << rsabocanec::descriptor::error_description(result) << '\n';
        }
        else {
            auto accept_result = server.accept();
            while (accept_result.has_value()) {
                std::thread([](rsabocanec::acceptor&& acceptor) {
                    std::cout << "New connection from " << acceptor.peer() << ':' << acceptor.peer_port() << '\n';

                    std::array<char, 24> request{};
                    auto io_result = acceptor.read(std::as_writable_bytes(std::span<char>(request)));
                    if (std::get<0>(io_result) != 0) {
                        std::cerr << "read failed; " << rsabocanec::descriptor::error_description(std::get<0>(io_result)) << '\n';
                    }
                    else {
                        const std::string_view response(request.cbegin(), std::get<1>(io_result));
                        std::cout << "Received '" << response << "'\n";

                        io_result = acceptor.write(std::as_bytes(std::span<const char>(response.cbegin(), response.cend())));
                        if (std::get<0>(io_result) != 0) {
                            std::cerr << "write failed; " << rsabocanec::descriptor::error_description(std::get<0>(io_result)) << '\n';
                        }
                        else {
                            std::cout << "Sent '" << response << "'\n";
                        }
                    }
                }, std::move(accept_result.value())).detach();

                accept_result = server.accept();
            }

            std::cerr << "accept failed: " << rsabocanec::descriptor::error_description(accept_result.error()) << '\n';
        }
    }

    std::cout << "EXIT\n";
    return EXIT_SUCCESS;
}
