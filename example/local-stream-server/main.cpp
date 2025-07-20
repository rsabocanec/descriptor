#include <local_stream_socket.h>

#include <array>
#include <thread>
#include <iostream>

#include <csignal>

namespace {
    volatile std::sig_atomic_t signal_status;

    rsabocanec::local_stream_socket *g_server{nullptr};
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

    constexpr std::string_view server_path{"/tmp/local"};

    rsabocanec::local_stream_socket server{};
    g_server = &server;

    auto result = server.bind(server_path);
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
                    std::cout << "New connection accepted\n";

                    std::array<char, 24> request{};
                    auto const [read_result, bytes_read] =
                        acceptor.read(std::as_writable_bytes(std::span<char>(request)));

                    if (read_result != 0) {
                        std::cerr << "read failed; " << rsabocanec::descriptor::error_description(read_result) << '\n';
                    }
                    else {
                        const std::string_view response(request.cbegin(), bytes_read);
                        std::cout << "Received '" << response << "'\n";

                        auto const [write_result, bytes_written] =
                            acceptor.write(std::as_bytes(std::span<const char>(response.cbegin(), response.cend())));

                        if (write_result != 0) {
                            std::cerr << "write failed; " << rsabocanec::descriptor::error_description(write_result) << '\n';
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

    ::unlink(server_path.data());

    return EXIT_SUCCESS;
}
