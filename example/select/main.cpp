#include <file.h>

#include "../utility.hpp"

#include <array>
#include <thread>
#include <iostream>

auto main(int argc, char **argv)->int {
    const std::string input_filename{"-i,--input"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {input_filename}, "Test select() function");

    const std::string filename{app->get_option(input_filename)->as<std::string>()};

    descriptor::reader reader{filename};

    if (!reader.valid()) {
        std::cerr << "Failed to open file /tmp/test-select\n";
        return EXIT_FAILURE;
    }

    using namespace std::chrono_literals;

    bool read_completed = false;

    while (!read_completed) {
        std::array<char, 1024> buffer{};

        auto const select_result = reader.select(100ms);

        switch (select_result) {
            case 0: {
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
                break;
            case ETIMEDOUT:
                std::cout << " . ";
                std::this_thread::sleep_for(1s);
                continue;
            default:
                std::cerr << "Select failed; " << descriptor::descriptor::error_description(select_result) << '\n';
                return EXIT_FAILURE;
        }
    }

    std::cout << "\nEXIT\n";
    return EXIT_SUCCESS;
}
