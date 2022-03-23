# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

# project name and languages
project(TextFileApp C RC)

##############################################################################

# TextFileApp.exe
add_executable(TextFileApp WIN32 TextFileApp.c TextFileApp_res.rc)
target_link_libraries(TextFileApp Common comctl32 shell32 shlwapi)

##############################################################################
