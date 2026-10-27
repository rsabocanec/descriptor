#include <file.h>

#include "../utility.hpp"

#include <array>
#include <span>

#include <cstring>

auto main(int argc, char** argv)->int {

    const std::string mmap_filename_option{"-m,--mmap-file"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {mmap_filename_option}, "Test memory-map");

    std::string filename{};

    try {
        filename = app->get_option("--mmap-file")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}\n", onf.what());
        return EXIT_FAILURE;
    }

    constexpr std::size_t new_file_size = 1024;

    {
        descriptor::file file{};

        auto result = file.open(filename);

        if (result) {
            logger->error("Failed to open file {} with result {}", filename, result.message());
            return result.value();
        }

        result = file.memory_map(descriptor::memory_map_access::write, new_file_size);
        if (result) {
            logger->error("Failed to memory map file {} with result {}", filename, result.message());
            return result.value();
        }

        // Write some data to the memory-mapped region
        constexpr std::string_view data = "Hello, memory-mapped file!";
        std::memcpy(file.memory_map_address(), data.data(), data.size());
    }

    // Print the content of the file to verify the write operation
    {
        descriptor::reader reader(filename);

        std::array<char, new_file_size> buffer{};
        auto const read_result = reader.read(buffer);
        if (!read_result) {
            logger->error("Failed to read from file {} with result {}",
                filename, read_result.error().message());
            return read_result.error().value();
        }

        fmt::println("{}",std::string_view(buffer.data(), read_result.value()));
    }

    return EXIT_SUCCESS;
}
