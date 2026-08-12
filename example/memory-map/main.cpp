#include <tcp_socket.h>
#include <file.h>

#include <array>
#include <span>

#include <iostream>

#include <cstring>

auto main(int argc, char** argv)->int {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " filename\n";
        return EXIT_FAILURE;
    }

    std::string_view filename{argv[1]};

    constexpr std::size_t new_file_size = 1024;

    {
        rsabocanec::file file{};

        auto result = file.open(filename);
        
        if (result != 0) {
            std::cerr << "Failed to open file " << filename << " with result " << result << ' '
                    << rsabocanec::descriptor::error_description(result) << '\n';
            return result;
        }

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
        rsabocanec::reader reader(filename);

        std::array<char, new_file_size> buffer{};
        auto [read_result, bytes_read] = reader.read(buffer);
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
