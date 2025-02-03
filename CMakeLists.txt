cmake_minimum_required(VERSION 3.10)
project(torrent_parser VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add library target for bencode
add_library(bencode
    src/bencode.cpp
    include/bencode.hpp
)

# Add library target for torrentfile
add_library(torrentfile
    src/torrentfile.cpp
    include/torrentfile.hpp
)

# Add executable
add_executable(torrent_parser src/main.cpp)

# Set include directories for libraries
target_include_directories(bencode PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(torrentfile PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Link bencode library to torrentfile
target_link_libraries(torrentfile
    PUBLIC
        bencode
)

# Link libraries to executable
target_link_libraries(torrent_parser
    PRIVATE
        torrentfile
)

# Add compiler warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(bencode PRIVATE -Wall -Wextra)
    target_compile_options(torrentfile PRIVATE -Wall -Wextra)
    target_compile_options(torrent_parser PRIVATE -Wall -Wextra)
endif()
