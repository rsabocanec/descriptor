#include <tcp_socket.h>

#include <array>
#include <span>

#include <iostream>

auto main()->int {
    rsabocanec::tcp_socket client{};

    auto result = client.connect("127.0.0.1", 9999);
    if (result != 0) {
        std::cerr << "connect failed; " << rsabocanec::descriptor::error_description(result) << '\n';
    }
    else {
        const std::string_view request{"Hello world!"};
        auto const [write_result, bytes_written] =
            client.write(request.cbegin(), request.cend());

        if (write_result != 0) {
            std::cerr << "write failed; " << rsabocanec::descriptor::error_description(write_result) << '\n';
        }
        else {
            std::cout << "Sent '" << request << "'\n";

            std::array<char, 24> response{};
            auto const [read_result, bytes_read] = client.read(response);
            if (read_result != 0) {
                std::cerr << "read failed; " << rsabocanec::descriptor::error_description(read_result) << '\n';
            }
            else {
                std::cout << "Received '" << std::string_view(response.cbegin(), bytes_read) << "'\n";
            }
        }
    }

    return EXIT_SUCCESS;
}
