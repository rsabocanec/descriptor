#include <file.h>

#include "../utility.hpp"

#include <array>
#include <span>

#include <cstring>
#include <cassert>

auto main(int argc, char** argv)->int {

    const std::string mem_filename_option{"-m,--mem-file"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {mem_filename_option}, "Test memory-file reader");

    std::string mem_filename{};

    try {
        mem_filename = app->get_option("--mem-file")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    constexpr std::size_t new_file_size = 1024;

    descriptor::memory_file mem_file_writer{};

    constexpr std::string_view data = "Hello, memory-mapped file!";

    {
    if (auto const open_result = mem_file_writer.open(mem_filename)) {
        logger->error("Failed to open memory file writer {} with result {}",
            mem_filename, open_result.message());
        return open_result.value();
    }

    auto const result = mem_file_writer.write(std::cbegin(data), std::cend(data));
    if (!result) {
        logger->error("Failed to write to memory file {} with result {}",
            mem_filename, result.error().message());
        return result.error().value();
    }

    fmt::println("Wrote {} bytes to memory file {}: {}", result.value(), mem_filename, data);

    fmt::print(fg(fmt::color::green) | fmt::emphasis::bold, "File size after writing is {} bytes\n", mem_file_writer.size());
    }

    {
    descriptor::memory_file mem_file_reader{std::move(mem_file_writer)};

    assert(mem_file_reader.valid());

    [[maybe_unused]] auto const reset_result = mem_file_reader.reset_position();
    assert(!reset_result);

    fmt::print(fg(fmt::color::green) | fmt::emphasis::bold, "File size before reading is {} bytes\n", mem_file_reader.size());

    std::array<char, new_file_size> buffer{};

    auto const result = mem_file_reader.read(buffer);
    if (!result) {
        logger->error("Failed to read from memory file {} with result {}",
            mem_filename, result.error().message());
        return result.error().value();
    }

    fmt::println("Read {} bytes from memory file {}: {}", result.value(), mem_filename,
            std::string_view(buffer.data(), result.value()));
    }

    return EXIT_SUCCESS;
}
