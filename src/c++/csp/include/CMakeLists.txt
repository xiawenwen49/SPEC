file(GLOB_RECURSE CURRENT_HEADERS *.hpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g2 -O3")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_CXX_STANDARD 11)


add_library(include_lib ${CURRENT_HEADERS})
set_target_properties(include_lib PROPERTIES LINKER_LANGUAGE CXX)