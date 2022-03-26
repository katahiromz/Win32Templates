# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

# project name and languages
project(RichTextFileApp C RC)

##############################################################################

# RichTextFileApp.exe
add_executable(RichTextFileApp WIN32
    AboutDlg.c
    CommandUI.c
    RichTextFileApp.c
    RichTextFileApp_res.rc)
target_link_libraries(RichTextFileApp Common comctl32 shell32 shlwapi gdiplus)
#target_compile_definitions(RichTextFileApp PRIVATE -DUNICODE -D_UNICODE)

##############################################################################
