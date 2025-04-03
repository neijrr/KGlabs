cmake_minimum_required(VERSION 3.10.0)
project(KGlab_1 VERSION 0.1.0 LANGUAGES C CXX)

set(CXX_STANDARD 20)

# Set target system
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(triple x86_64-pc-win32-gnu)

# Set compiler
set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER_TARGET ${triple})

# Target environment
set(CMAKE_FIND_ROOT_PATH "/usr/x86_64-w64-mingw32")

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl, -Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
add_link_options(-municode --allow-multiple-definition)

# Export config for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)


# add_executable(KGlab_1 Camera.cpp MyOGL.cpp Render.cpp main.cpp)
add_executable(KGlab_1 MyOGL.cpp Render.cpp main.cpp)
target_link_libraries(
    KGlab_1
    /home/alex/.wine/drive_c/windows/system32/opengl32.dll
    /home/alex/.wine/drive_c/windows/system32/gdi32.dll
    /home/alex/.wine/drive_c/windows/system32/glu32.dll
)
