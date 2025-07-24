#include <local_datagram_socket.h>

#include <array>

#include <iostream>
#include <memory>

auto main()->int {
    std::unique_ptr<rsabocanec::datagram_socket> client =
        std::make_unique<rsabocanec::local_datagram_socket>{};

    if (auto const result = client->open(); result != 0) {
        std::cerr   << "Failed to open UDP socket: " << result
                    << rsabocanec::descriptor::error_description(result)
                    << std::endl;
        return result;
    }

    constexpr std::string_view address = "/tmp/local-datagram-client";

    std::array<uint8_t, 4096> buffer{};

    std::string response{};

    while (response != "bye") {
        std::cout << "REQUEST: ";
        std::string request{};
        std::cin >> request;

        {
        auto [result, count] = client->write_to(request.cbegin(), request.cend(), address);

        if (result != 0) {
            std::cerr   << "Failed to write to "
                        << address
                        << " with result " << result << ' '
                        << rsabocanec::descriptor::error_description(result)
                        << std::endl;
            break;
        }
        }
        {
        std::string receive_address{};

        auto [result, count] =
            client->read_from(buffer, receive_address);

        if (result != 0) {
            std::cerr   << "Failed to read with result "
                        << result << ' '
                        << rsabocanec::descriptor::error_description(result)
                        << std::endl;
            break;
        }
        else {
            response = std::string(static_cast<const char*>(static_cast<const void*>(buffer.data())), count);
            std::cout << "Received " << response << '\n';
        }
        }
    }

    return EXIT_SUCCESS;
}
