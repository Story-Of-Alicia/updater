add_executable(manifester)
target_sources(manifester PRIVATE manifester.cpp)
target_link_libraries(manifester PRIVATE libpak z)
