# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

##############################################################################

# project name and languages
project(UnicodeConsoleApp C RC)

# UnicodeConsoleApp.exe
add_executable(UnicodeConsoleApp UnicodeConsoleApp.c UnicodeConsoleApp_res.rc)
target_link_libraries(UnicodeConsoleApp Common shlwapi)
target_compile_definitions(UnicodeConsoleApp PRIVATE -DUNICODE -D_UNICODE)

##############################################################################
