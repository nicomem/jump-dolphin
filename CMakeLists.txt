cmake_minimum_required(VERSION 3.17)
project(GPGPU-ICP LANGUAGES CXX)

include(FetchContent)

# External dependencies
FetchContent_Declare(CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG v1.9.1
    GIT_SHALLOW true
)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG v3.9.1
    GIT_SHALLOW true
)

message("Fetching 'CLI11'")
FetchContent_MakeAvailable(CLI11)
message("Fetching 'nlohmann_json'")
FetchContent_MakeAvailable(nlohmann_json)

# Print a warning if user did not set the Release mode
if (NOT CMAKE_BUILD_TYPE STREQUAL Release)
    message(WARNING "Add '-DCMAKE_BUILD_TYPE=Release' if you want an optimized build")
endif()

# C++ Properties
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wpedantic")

# Address sanitizer
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
# set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

# Source files
set(SOURCES
    # Relative to src/
    main.cpp
)
list(TRANSFORM SOURCES PREPEND "src/")

# Executable
add_executable(dolphin ${SOURCES})
target_link_libraries(dolphin PRIVATE CLI11 nlohmann_json::nlohmann_json)

# Set C++ macro for bench to locate files
target_compile_definitions(dolphin PRIVATE CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
