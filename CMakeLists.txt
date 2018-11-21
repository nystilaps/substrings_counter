SET(CMAKE_CXX_FLAGS "-std=c++11")

add_executable(counter main.cpp suffix_tree.cpp suffix_tree.h print.h print.cpp)
add_executable(test test.cpp suffix_tree.cpp suffix_tree.h print.h print.cpp)
add_executable(generator1 generator1.cpp)
add_executable(generator2 generator2.cpp)
