# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

# project name and languages
project(WindowApp C RC)

##############################################################################

# WindowApp.exe
add_executable(WindowApp WIN32 WindowApp.c WindowApp_res.rc)
target_link_libraries(WindowApp comctl32 shell32 shlwapi Common)
#target_compile_definitions(WindowApp PRIVATE -DUNICODE -D_UNICODE)

##############################################################################
