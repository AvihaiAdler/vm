cmake_minimum_required(VERSION 3.10)

project(lc3_vm VERSION 0.0.1)

add_compile_options(-Wall -Wextra -Wpedantic -std=c11 -o3 -Wconversion -Wunused-parameter)
add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)
add_link_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)

add_executable(vm src/vm.c src/operations.c src/image_loader.c)
target_include_directories(vm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
