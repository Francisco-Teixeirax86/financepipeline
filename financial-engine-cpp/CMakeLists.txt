cmake_minimum_required(VERSION 3.15)
project(stock_analysis_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the Homebrew directories for macOS
list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt" "/opt/homebrew/opt")

# Find required packages
find_package(CURL REQUIRED)

# For nlohmann_json, include the single header file approach
include(FetchContent)
FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add source files
set(SOURCE_FILESa
        src/main.cpp
        src/yahoo_finance_api.cpp
        src/trend_analyzer.cpp
        src/data_persistence.cpp
        src/stock_data_api.cpp
        src/trend_analyzer.cpp
        src/data_persistence.cpp
)

# Create executable
add_executable(stock_analyzer ${SOURCE_FILES})

# Link libraries
target_link_libraries(stock_analyzer
        PRIVATE
        CURL::libcurl
        nlohmann_json::nlohmann_json
        pthread
)

# Copy output directory to binary location
add_custom_command(TARGET stock_analyzer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_CURRENT_BINARY_DIR}/output
)