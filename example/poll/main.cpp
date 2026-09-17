#include <file.h>

#include <array>
#include <thread>
#include <iostream>

auto main()->int {
    descriptor::reader reader{"/tmp/test-poll"};

    if (!reader.valid()) {
        std::cerr << "Failed to open file /tmp/test-poll\n";
        return EXIT_FAILURE;
    }

    using namespace std::chrono_literals;

    bool read_completed = false;

    while (!read_completed) {
        std::array<char, 1024> buffer{};

        descriptor::polled_state state{};

        auto const poll_result = reader.poll(100ms, state);


        switch (poll_result) {
            case 0: 
                if (state.has_error()) {
                    std::cerr << "\nPoll error detected!\n";
                    return EXIT_FAILURE;
                }
                else if (state.has_something_to_read()) {
                    auto const [read_error, bytes_read] = reader.read(buffer);
                    if (read_error != 0) {
                        std::cerr << "\nRead failed; " << descriptor::descriptor::error_description(read_error) << '\n';
                        return EXIT_FAILURE;
                    }
                    else if (bytes_read == 0) {
                        std::cout << "\nRead completed!\n";
                        read_completed = true;
                    }
                    else {
                        std::string_view read_bytes(buffer.cbegin(), bytes_read);
                        std::cout   << "\nRead: " 
                                    << std::string_view (buffer.cbegin(), bytes_read) << "\n";
                    }
                }
                else if (state.has_hangup()) {
                    std::cout << "\nFile closed!\n";
                    return EXIT_SUCCESS;
                }
                break;
            case ETIMEDOUT:
                std::cout << " . ";
                std::this_thread::sleep_for(1s);
                continue;
            default:
                std::cerr << "Poll failed; " << descriptor::descriptor::error_description(poll_result) << '\n';
                return EXIT_FAILURE;
        }
    }

    std::cout << "\nEXIT\n";
    return EXIT_SUCCESS;
}
