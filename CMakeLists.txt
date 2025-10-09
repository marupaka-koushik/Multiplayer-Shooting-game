cmake_minimum_required(VERSION 3.16)
project(MiniMilitiaClone)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find raylib
find_package(raylib QUIET)
if (NOT raylib_FOUND)
    # If not found, try pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(RAYLIB REQUIRED raylib)
endif()

# Server executable (no graphics needed)
add_executable(server
    server.cpp
)

# Client executable (with raylib graphics)
add_executable(client
    client.cpp
)

# Link raylib to client only
if (raylib_FOUND)
    target_link_libraries(client raylib)
else()
    target_link_libraries(client ${RAYLIB_LIBRARIES})
    target_include_directories(client PRIVATE ${RAYLIB_INCLUDE_DIRS})
endif()

# Platform-specific networking libraries
if(WIN32)
    target_link_libraries(server ws2_32)
    target_link_libraries(client ws2_32)
endif()

# macOS specific frameworks for raylib
if(APPLE)
    target_link_libraries(client "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
endif()
