#include <file.h>

#include "../utility.hpp"

#include <array>
#include <thread>

auto main(int argc, char **argv)->int {
    const std::string input_filename{"-i,--input"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {input_filename}, "Test poll() function");

    std::string filename{};

    try {
        filename = app->get_option("--input")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}", onf.what());
        return EXIT_FAILURE;
    }

    descriptor::reader reader{filename};

    if (!reader.valid()) {
        logger->error("Failed to open file {}", filename);
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
                    logger->error("Poll error detected!");
                    return EXIT_FAILURE;
                }
                else if (state.has_something_to_read()) {
                    auto const [read_error, bytes_read] = reader.read(buffer);
                    if (read_error != 0) {
                        logger->error("Read failed; {}", descriptor::error_description(read_error));
                        return EXIT_FAILURE;
                    }
                    else if (bytes_read == 0) {
                        logger->info("Read completed!");
                        read_completed = true;
                    }
                    else {
                        logger->info("Read: {}", std::string_view(buffer.cbegin(), bytes_read));
                    }
                }
                else if (state.has_hangup()) {
                    logger->info("Input file closed!");
                    read_completed = true;
                }
                break;
            case ETIMEDOUT:
                fmt::print(fg(fmt::color::yellow), " . ");
                std::this_thread::sleep_for(1s);
                continue;
            default:
                logger->error("Poll failed; {}", descriptor::error_description(poll_result));
                return EXIT_FAILURE;
        }
    }

    fmt::print( fg(fmt::color::green), "\nEXIT!\n");
    return EXIT_SUCCESS;
}
