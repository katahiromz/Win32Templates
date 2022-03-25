# libCommon.a
add_library(Common STATIC
    Base64.c
    Bitmap.cpp
    Framework.c
    Recent.c
    Shortcut.c
    Utils.c
)
#target_compile_definitions(Common PRIVATE -DUNICODE -D_UNICODE)
