cmake_minimum_required(VERSION 3.14)
project(FMSVPipeline)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the Homebrew-installed packages
find_package(nlohmann_json REQUIRED)
find_package(cpr REQUIRED)

add_executable(pipeline_s1s2 pipeline_s1s2.cpp)

# Link them to your executable
target_link_libraries(pipeline_s1s2 PRIVATE cpr::cpr nlohmann_json::nlohmann_json)