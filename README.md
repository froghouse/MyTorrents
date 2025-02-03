# BitTorrent Metadata Parser (MyTorrents)

A modern C++17 library for parsing and analyzing BitTorrent metadata files (`.torrent` files). This project provides a robust implementation of the Bencode parser and torrent file analyzer with a clean, type-safe interface.

## Features

- Complete Bencode format parser supporting all data types:
  - Integers (e.g., `i42e`)
  - Strings (e.g., `4:spam`)
  - Lists (e.g., `l4:spami42ee`)
  - Dictionaries (e.g., `d3:foo3:bare`)
- Comprehensive `.torrent` file metadata extraction
- Modern C++17 implementation using type-safe containers
- Exception-based error handling with detailed error messages
- Memory-safe design using smart pointers
- CMake-based build system

## Requirements

- C++17 compatible compiler
- CMake 3.10 or higher
- Standard C++ library

## Building the Project

```bash
# Create a build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# The build will produce:
# - libbencode.a (Bencode parser library)
# - libtorrentfile.a (Torrent metadata parser library)
# - torrent_parser (Example executable)
```

## Usage Example

```cpp
#include <iostream>
#include <torrentfile.hpp>

int main() {
    try {
        // Load and parse a .torrent file
        TorrentFile torrent("example.torrent");

        // Access torrent metadata
        std::cout << "Name: " << torrent.getName() << '\n';
        std::cout << "Total Size: " << torrent.getTotalSize() << " bytes\n";
        
        // List all files in the torrent
        for (const auto& file : torrent.getFiles()) {
            std::cout << file.path << " (" << file.length << " bytes)\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

## API Overview

### BencodeValue Class
The `BencodeValue` class represents a Bencode-encoded value and provides type-safe access to its contents:

```cpp
class BencodeValue {
public:
    // Constructors for different value types
    BencodeValue(int64_t i);
    BencodeValue(const std::string& s);
    BencodeValue(List&& l);
    BencodeValue(Dict&& d);

    // Type checking methods
    bool isInt() const;
    bool isString() const;
    bool isList() const;
    bool isDict() const;

    // Value getters
    int64_t getInt() const;
    const std::string& getString() const;
    const List& getList() const;
    const Dict& getDict() const;
};
```

### TorrentFile Class
The `TorrentFile` class provides access to parsed torrent metadata:

```cpp
class TorrentFile {
public:
    explicit TorrentFile(const std::string& filepath);

    // Metadata accessors
    const std::string& getAnnounce() const;
    const std::string& getName() const;
    int64_t getPieceLength() const;
    const std::vector<std::string>& getPieces() const;
    const std::vector<FileInfo>& getFiles() const;
    int64_t getTotalSize() const;
    bool isSingleFile() const;
};
```

## Error Handling

The library uses exceptions to handle error conditions. Common exceptions include:

- `std::runtime_error`: Invalid file format, parsing errors
- `std::bad_variant_access`: Type mismatch when accessing values
- `std::ios_base::failure`: File I/O errors

Example error handling:

```cpp
try {
    TorrentFile torrent("example.torrent");
    // ... use torrent object
} catch (const std::runtime_error& e) {
    // Handle parsing/format errors
} catch (const std::ios_base::failure& e) {
    // Handle I/O errors
} catch (const std::exception& e) {
    // Handle other errors
}
```

## Project Structure

```
.
├── include/
│   ├── bencode.hpp      # Bencode parser declarations
│   └── torrentfile.hpp  # Torrent file parser declarations
├── src/
│   ├── bencode.cpp      # Bencode parser implementation
│   ├── torrentfile.cpp  # Torrent file parser implementation
│   └── main.cpp         # Example program
└── CMakeLists.txt      # Build configuration
```

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit pull requests. For major changes, please open an issue first to discuss what you would like to change.
