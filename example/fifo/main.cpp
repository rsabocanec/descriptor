#include <file.h>

#include "../utility.hpp"

#include <array>
#include <chrono>
#include <thread>

#include <csignal>

namespace {
    volatile std::sig_atomic_t signal_status;

    std::atomic_bool run{true};
    std::string fifo_path{};
}

void signal_handler(int signal) {
    signal_status = signal;

    using namespace std::chrono_literals;

    switch (signal) {
        case SIGTERM:
	    case SIGINT:
            run = false;
	        ::unlink(fifo_path.c_str());
            break;
        default:
            break;
    }
}


auto main(int argc, char **argv)->int {

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    const std::string filename_option{"-f,--fifo"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {filename_option}, "Test make_fifo() function");

    try {
        fifo_path = app->get_option("--fifo")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    if (auto const result = descriptor::make_fifo(fifo_path); result != 0) {
        logger->error("Failed to create fifo file {}, with error {} {}!", 
            fifo_path, result, descriptor::error_description(result));
        return EXIT_FAILURE;
    }

    while (run.load()) {
        fmt::print("REQUEST: ");
    
        std::string request{};
        std::cin >> request;

        descriptor::writer fifo_writer(fifo_path);

        if (!fifo_writer.valid()) {
            logger->error("Failed to open file {} for writing", fifo_path);
            run = false;
        }
        else {
            auto const [result, count] = fifo_writer.write(std::cbegin(request), std::cend(request));
            if (result != 0) {
                logger->error("Failed to write to '{}' fifo, with error {} {}",
                    fifo_path, result, descriptor::error_description(result));

                run = false;
            }
            else {
                logger->info("Wrote {} bytes to the '{}' fifo file", count, fifo_path);
            }
        }
    }

    fmt::print( fg(fmt::color::green), "\nEXIT!\n");
    return EXIT_SUCCESS;
}
