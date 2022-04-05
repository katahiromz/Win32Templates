# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

# project name and languages
project(DialogApp++ CXX RC)

##############################################################################

# DialogApp++.exe
add_executable(DialogApp++ WIN32 DialogApp.cpp DialogApp_res.rc)
target_link_libraries(DialogApp++ Common comctl32 shell32 shlwapi)

##############################################################################
