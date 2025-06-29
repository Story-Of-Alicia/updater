cmake_minimum_required(VERSION 3.31)
project(alicia_updater)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fshort-wchar)

include_directories(lib/libpak/include)

add_subdirectory(lib/libpak)
add_subdirectory(lib/libupdate)
add_subdirectory(src/tooling)
add_subdirectory(src/updater)