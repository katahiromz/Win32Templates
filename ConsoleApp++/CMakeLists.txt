# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

##############################################################################

if(WIN32)
    # project name and languages
    project(ConsoleApp++ CXX RC)

    # ConsoleApp++.exe
    add_executable(ConsoleApp++ ConsoleApp.cpp ConsoleApp_res.rc)
    target_link_libraries(ConsoleApp++ Common)
else()
    # project name and languages
    project(ConsoleApp++ CXX)

    # ConsoleApp++.exe
    add_executable(ConsoleApp++ ConsoleApp.cpp)
    target_link_libraries(ConsoleApp++)
endif()

##############################################################################
