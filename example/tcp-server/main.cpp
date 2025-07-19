#include <tcp_socket.h>

#include <array>
#include <iostream>
#include <span>

#include "../../socket.h"

auto main()->int {
    rsabocanec::tcp_socket server{};

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
            if (accept_result.has_value()) {
                auto &acceptor = accept_result.value();

                std::array<char, 24> request{};
                auto io_result = server.read(std::as_writable_bytes(std::span<char>(request)));
                if (std::get<0>(io_result) != 0) {
                    std::cerr << "read failed; " << rsabocanec::descriptor::error_description(std::get<0>(io_result)) << '\n';
                }
                else {
                    const std::string_view response{"Hello world!"};
                    auto io_result = server.write(std::as_bytes(std::span<const char>(response.cbegin(), response.cend())));
                    if (std::get<0>(io_result) != 0) {
                        std::cerr << "write failed; " << rsabocanec::descriptor::error_description(std::get<0>(io_result)) << '\n';
                    }
                }
            }
            else {
                std::cerr << "accept failed: " << rsabocanec::descriptor::error_description(accept_result.error()) << '\n';
            }
        }
    }

    return EXIT_SUCCESS;
}
