cmake_minimum_required(VERSION 2.8)
include(ExternalProject)

project(csp_server)

# Find default python libraries and interpreter
# find_package(PythonInterp REQUIRED)
# find_package(PythonLibs REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_PREFIX_PATH "LIBTORCH_DIR")
find_package(Torch REQUIRED)

include_directories(${CONDA_PYTHON_INCLUDE_DIR}
"REPO_DIR_ON_YOUR_MACHINE/src/c++"
"REPO_DIR_ON_YOUR_MACHINE/src/c++/third_libs"
"REPO_DIR_ON_YOUR_MACHINE/src/c++/third_libs/fmt/include"

)

link_directories(
${BOOST_LIB_DIR}
)

add_subdirectory("REPO_DIR_ON_YOUR_MACHINE/src/c++/csp/include")

add_executable(cspserver cspserver.cpp)
target_link_libraries(cspserver pthread include_lib libfmt.a "${TORCH_LIBRARIES}")
set_property(TARGET cspserver PROPERTY CXX_STANDARD 14) # must be c++14