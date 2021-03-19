cmake_minimum_required(VERSION 3.10)
project(libusb)

set(SOURCES
    libusb/libusb.h 
    libusb/libusbi.h
    libusb/version.h 
    libusb/version_nano.h
	libusb/core.c 
    libusb/descriptor.c 
    libusb/hotplug.h 
    libusb/hotplug.c 
    libusb/io.c 
    libusb/strerror.c 
    libusb/sync.c
)

set(HEADER_PATHS
    libusb
    libusb/os
)

if (WIN32)
    list(APPEND SOURCES
        libusb/os/threads_windows.h
        libusb/os/threads_windows.c
        libusb/os/events_windows.h
        libusb/os/events_windows.c
        libusb/libusb-1.0.def
        libusb/libusb-1.0.rc
        libusb/os/windows_common.h
        libusb/os/windows_common.c
        libusb/os/windows_usbdk.h
        libusb/os/windows_usbdk.c
        libusb/os/windows_winusb.h
        libusb/os/windows_winusb.c
        msvc/config.h
    )
    
    list(APPEND HEADER_PATHS
        msvc
    )
elseif (ANDROID)
    list(APPEND SOURCES
        libusb/os/threads_posix.h
        libusb/os/threads_posix.c
        libusb/os/events_posix.h
        libusb/os/events_posix.c
        libusb/os/linux_netlink.c
        libusb/os/linux_usbfs.h
        libusb/os/linux_usbfs.c
        android/config.h
    )

    list(APPEND HEADER_PATHS
        android
    )
elseif (APPLE)
    list(APPEND SOURCES
        libusb/os/threads_posix.h
        libusb/os/threads_posix.c
        libusb/os/events_posix.h
        libusb/os/events_posix.c
        libusb/os/darwin_usb.c
        libusb/os/darwin_usb.h
        Xcode/config.h
    )

    list(APPEND HEADER_PATHS
        Xcode
    )
else()
    #set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-ludev")
    list(APPEND SOURCES
        libusb/os/threads_posix.h
        libusb/os/threads_posix.c
        libusb/os/events_posix.h
        libusb/os/events_posix.c
        libusb/os/linux_netlink.c
        libusb/os/linux_udev.c
        libusb/os/linux_usbfs.h
        libusb/os/linux_usbfs.c
    )
endif()

add_library(libusb SHARED ${SOURCES})
target_include_directories(libusb PUBLIC ${HEADER_PATHS})
install(DIRECTORY "libusb/" DESTINATION include FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*impl*.*" EXCLUDE)
install(TARGETS libusb
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)