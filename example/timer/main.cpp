#include <timer.h>

#include "../utility.hpp"

auto main(int argc, char** argv) -> int {
    using namespace std::chrono_literals;

    {
    fmt::print(fg(fmt::color::orange), "Timer example #1\n");

    descriptor::timer t;
    auto result = t.arm(1s);

    if (result) {
        descriptor::report_error(std::cerr, result);
    }

    for (auto i = 0; i < 10; ++i) {
        result = t.wait();

        if (result) {
            descriptor::report_error(std::cerr, result);
        }
        else {
            static int32_t counter = 0;
            fmt::print(fg(fmt::color::yellow), "{}\n", ++counter);
        }
    }

    result = t.disarm();

    if (result) {
        descriptor::report_error(std::cerr, result);
    }
    }

    {
    fmt::print(fg(fmt::color::orange), "Timer example #2\n");
    descriptor::timer t;

    std::promise<std::errc> promise{};
    auto future = promise.get_future();

    t.arm(1s, std::move(promise), []() {
        static int32_t counter = 0;
        fmt::print(fg(fmt::color::yellow), "{}\n", ++counter);
    });

    std::this_thread::sleep_for(7500ms);

    fmt::print(fg(fmt::color::orange), "Waited 7.5s, wait for timer thread to finish\n");

    fmt::print(fg(fmt::color::orange), "Rearming timer for 2.5s\n");

    auto result = t.arm(2500ms);

    if (result) {
        descriptor::report_error(std::cerr, result);
    }

    std::this_thread::sleep_for(7400ms);

    fmt::print(fg(fmt::color::orange), "Waited 7.4s, wait for timer thread to execute\n");

    fmt::print(fg(fmt::color::orange), "Rearming timer for 0.5s\n");

    result = t.arm(500ms);

    if (result) {
        descriptor::report_error(std::cerr, result);
    }

    std::this_thread::sleep_for(4900ms);

    fmt::print(fg(fmt::color::orange), "Waited 4.9s, wait for timer thread to execute\n");

    fmt::print(fg(fmt::color::orange), "Disarming timer\n");

    result = t.disarm();

    if (result) {
        descriptor::report_error(std::cerr, result);
    }

    fmt::print(fg(fmt::color::orange), "Waiting for timer thread to finish!\n");

    result = std::make_error_code(future.get());

    fmt::print(fg(fmt::color::orange), "Timer thread finished\n");

    if (result) {
        descriptor::report_error(std::cerr, result);
    }
    }

    {
    fmt::print(fg(fmt::color::orange), "Deadline example\n");
    descriptor::deadline dl;

    std::promise<std::errc> promise{};
    auto future = promise.get_future();

    dl.arm(5s, std::move(promise), []() {
        fmt::print(fg(fmt::color::yellow), "Deadline expired after 5 seconds!\n");
    });

    fmt::print(fg(fmt::color::orange), "Waiting for deadline result!\n");

    auto const result = std::make_error_code(future.get());

    fmt::print(fg(fmt::color::orange), "Deadline expired\n");

    if (result) {
        descriptor::report_error(std::cerr, result);
    }
    }

    return EXIT_SUCCESS;
}