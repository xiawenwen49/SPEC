cmake_minimum_required(VERSION 3.12)
project(SPEC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g2 -lstdc++fs -O3")
set(CMAKE_PREFIX_PATH "LIBTORCH_DIR")

set(CMAKE_CXX_STANDARD 11)
find_package(Torch REQUIRED)


include_directories("REPO_DIR_ON_YOUR_MACHINE/src/c++/" 
"REPO_DIR_ON_YOUR_MACHINE/src/c++/third_libs"
"REPO_DIR_ON_YOUR_MACHINE/src/c++/third_libs/fmt/include"
)
link_directories(
"REPO_DIR_ON_YOUR_MACHINE/src/c++/third_libs/fmt/build"
)

add_definitions(-DCSP=1)
add_subdirectory(include)


add_executable(csp main.cpp)
set_target_properties(csp PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(csp include_lib libfmt.a "${TORCH_LIBRARIES}")
set_property(TARGET csp PROPERTY CXX_STANDARD 14) # must be c++14