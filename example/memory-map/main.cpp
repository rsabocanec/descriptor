#include <tcp_socket.h>
#include <file.h>

#include <array>
#include <span>

#include <iostream>

#include <cstring>

#include <fcntl.h>

auto main(int argc, char** argv)->int {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " filename\n";
        return EXIT_FAILURE;
    }

    std::string_view filename{argv[1]};

    {
        rsabocanec::file file{};

        auto result = file.open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IROTH);
        if (result != 0) {
            std::cerr << "Failed to open file " << filename << " with result " << result << ' '
                    << rsabocanec::descriptor::error_description(result) << '\n';
            return result;
        }

        constexpr std::size_t new_file_size = 1024;

        result = file.memory_map(rsabocanec::memory_map_access::write, new_file_size);
        if (result != 0) {
            std::cerr << "Failed to memory map file " << filename << " with result " << result << ' '
                    << rsabocanec::descriptor::error_description(result) << '\n';
            return result;
        }

        // Write some data to the memory-mapped region
        std::string_view data = "Hello, memory-mapped file!";
        std::memcpy(file.memory_map_address(), data.data(), data.size());
    }
    
    // Print the content of the file to verify the write operation
    {
        rsabocanec::file file{};
        auto result = file.open(filename, O_RDONLY);
        if (result != 0) {
            std::cerr << "Failed to open file " << filename << " with result " << result << ' '
                      << rsabocanec::descriptor::error_description(result) << '\n';
            return result;
        }

        std::array<char, 1024> buffer{};
        auto [read_result, bytes_read] = file.read(buffer);
        if (read_result != 0) {
            std::cerr << "Failed to read from file " << filename << " with result " << read_result << ' '
                      << rsabocanec::descriptor::error_description(read_result) << '\n';
            return read_result;
        }

        std::cout.write(buffer.data(), bytes_read);
        std::cout << '\n';
    }

    return EXIT_SUCCESS;
}
