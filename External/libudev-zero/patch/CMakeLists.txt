project(libudev-zero C)

set(CMAKE_C_STANDARD 11)

file(GLOB sources "*.h" "*.c")

add_library(udev-zero-static STATIC ${sources})
install(TARGETS udev-zero-static EXPORT udev-zero ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES udev.h DESTINATION include)