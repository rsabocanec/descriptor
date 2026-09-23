#include <tcp_socket.h>
#include <file.h>

#include "../utility.hpp"

#include <array>
#include <span>

#include <cstring>

auto main(int argc, char** argv)->int {

    const std::string mmap_filename_option{"-m,--mmap-file"};

    auto [logger, app] = descriptor::example::utility::init_app(
        argc, argv, {mmap_filename_option}, "Test poll() function");

    std::string filename{};

    try {
        filename = app->get_option("--mmap-file")->as<std::string>();
    }
    catch (const CLI::OptionNotFound &onf) {
        logger->critical(onf.what());
        fmt::print(fg(fmt::color::crimson), "{}", onf.what());
        return EXIT_FAILURE;
    }

    constexpr std::size_t new_file_size = 1024;

    {
        descriptor::file file{};

        auto result = file.open(filename);
        
        if (result != 0) {
            logger->error("Failed to open file {} with result {}", 
                filename, descriptor::error_description(result));
            return result;
        }

        result = file.memory_map(descriptor::memory_map_access::write, new_file_size);
        if (result != 0) {
            logger->error("Failed to memory map file {} with result {}", filename, descriptor::error_description(result));
            return result;
        }

        // Write some data to the memory-mapped region
        std::string_view data = "Hello, memory-mapped file!";
        std::memcpy(file.memory_map_address(), data.data(), data.size());
    }
    
    // Print the content of the file to verify the write operation
    {
        descriptor::reader reader(filename);

        std::array<char, new_file_size> buffer{};
        auto [read_result, bytes_read] = reader.read(buffer);
        if (read_result != 0) {
            logger->error("Failed to read from file {} with result {}", filename, descriptor::error_description(read_result));
            return read_result;
        }

        fmt::println("{}",std::string_view(buffer.data(), bytes_read));
    }

    return EXIT_SUCCESS;
}
