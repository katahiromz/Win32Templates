# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

# project name and languages
project(Win32Templates C CXX RC)

##############################################################################

add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(Common)
include_directories(Common)

add_subdirectory(ConsoleApp)
add_subdirectory(UnicodeConsoleApp)
add_subdirectory(ConsoleApp++)
add_subdirectory(UnicodeConsoleApp++)
add_subdirectory(DialogApp++)
add_subdirectory(WindowApp)
add_subdirectory(TextFileApp)
add_subdirectory(RichTextFileApp)
