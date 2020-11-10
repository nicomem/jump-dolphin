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
FetchContent_Declare(cpr
    GIT_REPOSITORY https://github.com/whoshuu/cpr
    # Hash for the 2020-11-10 master
    # 1.5.2 requires curl PROTOCOLS but master does not
    # -> could not use system curl lib before (in most cases)
    GIT_TAG 47cc92045247c429037324cb4eee67ede81c35cf
    GIT_SHALLOW true
)
FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 7.1.1
    GIT_SHALLOW true
)
FetchContent_Declare(date
    GIT_REPOSITORY https://github.com/HowardHinnant/date
    GIT_TAG v3.0.0
    GIT_SHALLOW true
)

set(BUILD_CPR_TESTS OFF)
set(BUILD_CPR_TESTS_SSL OFF)
set(USE_SYSTEM_CURL ON)

message("Fetching 'CLI11'")
FetchContent_MakeAvailable(CLI11)
message("Fetching 'nlohmann_json'")
FetchContent_MakeAvailable(nlohmann_json)
message("Fetching 'cpr'")
FetchContent_MakeAvailable(cpr)
message("Fetching 'fmt'")
FetchContent_MakeAvailable(fmt)
message("Fetching 'date'")
FetchContent_MakeAvailable(date)

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
    main.cpp

    finmath.cpp
    finmath.hpp
    save_data.cpp
    save_data.hpp

    jump/client.hpp
    jump/private_client.cpp
    jump/private_client.hpp
    jump/types_json_light.hpp
    jump/types_json.hpp
    jump/types.hpp
)
list(TRANSFORM SOURCES PREPEND "src/")

# Executable
add_executable(dolphin ${SOURCES})
target_link_libraries(dolphin PRIVATE 
    CLI11
    nlohmann_json::nlohmann_json
    cpr::cpr
    fmt::fmt
    date::date
)

# Set C++ macro for bench to locate files
target_compile_definitions(dolphin PRIVATE CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
