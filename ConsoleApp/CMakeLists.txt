# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)


##############################################################################

# ConsoleApp.exe
if(WIN32)
    # project name and languages
    project(ConsoleApp C RC)

    add_executable(ConsoleApp ConsoleApp.c ConsoleApp_res.rc)
    target_link_libraries(ConsoleApp Common shlwapi)
else()
    # project name and languages
    project(ConsoleApp C)

    add_executable(ConsoleApp ConsoleApp.c)
    target_link_libraries(ConsoleApp)
endif()

##############################################################################
