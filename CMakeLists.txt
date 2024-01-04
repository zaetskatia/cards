cmake_minimum_required(VERSION 3.0)

project(cardsServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add all source files to a variable
file(GLOB SOURCES "*.cpp")

# Specify the target executable and its source files
add_executable(CardsServer ${SOURCES})

find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(SQLiteCpp)

# Link libraries
target_link_libraries(CardsServer stdc++ boost_system boost_thread SQLiteCpp pthread nlohmann_json::nlohmann_json)
