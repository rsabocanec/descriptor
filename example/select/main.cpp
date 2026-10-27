#include <file.h>

#include "../utility.hpp"

#include <array>
#include <thread>

auto main(int argc, char **argv)->int {
    const std::string input_filename{"-i,--input"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {input_filename}, "Test select() function");

    std::string filename{};

    try {
        filename = app->get_option("--input")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
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

        auto const select_result = reader.select(100ms);

        if (!select_result) {
            auto const read_result = reader.read(buffer);
            if (!read_result) {
                logger->error("Read failed; {}", read_result.error().message());
                return EXIT_FAILURE;
            }
            else if (read_result.value() == 0) {
                logger->info("Read completed!");
                read_completed = true;
            }
            else {
                logger->info("Read: {}", std::string_view(buffer.cbegin(), read_result.value()));
            }
        }
        else {
            switch (select_result.value()) {
                case static_cast<int>(std::errc::timed_out):
                    fmt::print(" . ");
                    std::this_thread::sleep_for(1s);
                    break;
                default:
                    logger->error("Select failed; {}", select_result.message());
                    return EXIT_FAILURE;
            }
        }
    }

    fmt::print("\nEXIT\n");
    return EXIT_SUCCESS;
}
