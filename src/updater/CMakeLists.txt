add_executable(updater)
target_sources(updater PRIVATE updater.cpp)
target_link_libraries(updater PRIVATE libupdate z ssl crypto libpak)
