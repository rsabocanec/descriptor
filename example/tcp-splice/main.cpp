#include <tcp_socket.h>
#include <file.h>

#include <array>
#include <span>

#include <iostream>

#include <fcntl.h>

auto main(int argc, char** argv)->int {

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " filename address port\n";
        return EXIT_FAILURE;
    }

    std::string_view filename{argv[1]};

    rsabocanec::file file{};

    auto result = file.open(filename, O_RDONLY);
    if (result != 0) {
        std::cerr << "Failed to open file " << filename << " with result " << result << ' '
                  << rsabocanec::descriptor::error_description(result) << '\n';
        return result;
    }

    std::string_view address{argv[2]};
    auto const port = static_cast<uint16_t>(std::stoul(argv[3]));
    
    rsabocanec::tcp_socket client{};

    result = client.connect(address, port);
    if (result != 0) {
        std::cerr << "Connect to " << address << ':' << port << " failed;\n"
                  << rsabocanec::descriptor::error_description(result) << '\n';
    }
    else {
        auto const [splice_result, bytes_spliced] = client.splice(file, file.size());

        if (splice_result != 0) {
            std::cerr << "Splice failed; " << rsabocanec::descriptor::error_description(splice_result) << '\n';
        }
        else {
            std::cout << "Spliced " << bytes_spliced << " bytes from file " << filename
                      << " to TCP socket " << address << ':' << port << '\n';
        }

        std::cout << "Disconnecting!\n";

        result = client.disconnect();
        if (result != 0) {
            std::cerr << "Disconnect failed; " << rsabocanec::descriptor::error_description(result) << '\n';
        }
    }

    result = file.close();
    if (result != 0) {
        std::cerr << "Failed to close file " << filename << " with result " << result << ' '
                  << rsabocanec::descriptor::error_description(result) << '\n';
    }

    return EXIT_SUCCESS;
}
