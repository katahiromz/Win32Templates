# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

# project name and languages
project(PanelApp C RC)

##############################################################################

# PanelApp.exe
add_executable(PanelApp WIN32
	Canvas.c
	Panel1.c
	Panel2.c
	Panel3.c
	PanelApp.c PanelApp_res.rc
)
target_link_libraries(PanelApp Common comctl32 shell32 shlwapi)
#target_compile_definitions(PanelApp PRIVATE -DUNICODE -D_UNICODE)

##############################################################################
