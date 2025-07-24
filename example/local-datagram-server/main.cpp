#include <local_datagram_socket.h>

#include <array>

#include <iostream>

auto main()->int {
    std::unique_ptr<rsabocanec::datagram_socket> server =
        std::make_unique<rsabocanec::local_datagram_socket>{};

    if (auto const result = server->open(); result != 0) {
        std::cerr   << "Failed to open UDP socket: " << result
                    << rsabocanec::descriptor::error_description(result)
                    << std::endl;
        return result;
    }

    constexpr std::string_view address = "/tmp/local-datagram-server";

    if (auto const result = server->bind(address); result != 0) {
        std::cerr   << "Failed to bind to " << address
                    << " with result " << result
                    << rsabocanec::descriptor::error_description(result)
                    << std::endl;
        return result;
    }

    std::array<uint8_t, 4096> buffer{};

    std::string request{};

    while (request != "bye") {
        std::string receive_address{};

        {
        auto [result, count] =
            server->read_from(buffer, receive_address);

        if (result != 0) {
            std::cerr   << "Failed to read with result "
                        << result << ' '
                        << rsabocanec::descriptor::error_description(result)
                        << std::endl;
            break;
        }
        else {
            request = std::string(static_cast<const char*>(static_cast<const void*>(buffer.data())), count);
            std::cout << "Received " << request << '\n';
        }
        }
        {
        auto [result, count] =
            server->write_to(request.cbegin(), request.cend(), receive_address);

        if (result != 0) {
            std::cerr   << "Failed to write to "
                        << receive_address
                        << " with result " << result << ' '
                        << rsabocanec::descriptor::error_description(result)
                        << std::endl;
            break;
        }
        }
    }
    return EXIT_SUCCESS;
}
