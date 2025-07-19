#include <tcp_socket.h>

#include <array>
#include <iostream>
#include <span>

auto main()->int {
    rsabocanec::tcp_socket client{};

    auto result = client.connect("127.0.0.1", 9999);
    if (result != 0) {
        std::cerr << "connect failed; " << rsabocanec::descriptor::error_description(result) << '\n';
    }
    else {
        const std::string_view request{"Hello world!"};
        auto io_result = client.write(std::as_bytes(std::span<const char>(request.cbegin(), request.cend())));
        if (std::get<0>(io_result) != 0) {
            std::cerr << "write failed; " << rsabocanec::descriptor::error_description(std::get<0>(io_result)) << '\n';
        }
        else {
            std::cout << "Sent '" << request << "'\n";

            std::array<char, 24> response{};
            io_result = client.read(std::as_writable_bytes(std::span<char>(response)));
            if (std::get<0>(io_result) != 0) {
                std::cerr << "read failed; " << rsabocanec::descriptor::error_description(std::get<0>(io_result)) << '\n';
            }
            else {
                std::cout << "Received '" << std::string_view(response.cbegin(), std::get<1>(io_result)) << "'\n";
            }
        }
    }

    return EXIT_SUCCESS;
}
