# cmake_minimum_required(VERSION 3.7)

# project(cartserver)

# set(CMAKE_CXX_STANDARD 20)
# set(THREADS_PREFER_PTHREAD_FLAG ON)

# find_package(Boost COMPONENTS system filesystem REQUIRED)
# find_package(Threads)

# #include_directories(${Boost_INCLUDE_DIRS})
# #add_executable(cartserver main.cpp)
# #target_link_libraries(cartserver ${Boost_LIBRARIES} Threads::Threads)


cmake_minimum_required(VERSION 3.7)

project(cartserver)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Threads)

# Include directories for MySQL Connector/C++
include_directories(${Boost_INCLUDE_DIRS})

# Add your source files
add_executable(cartserver main.cpp)

# Link against Boost, Threads, and MySQL Connector/C++
target_link_libraries(cartserver ${Boost_LIBRARIES} Threads::Threads /usr/lib/x86_64-linux-gnu/libsqlite3.so.0)