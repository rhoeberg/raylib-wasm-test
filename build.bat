emcc -o game.html main.cpp -Os -Wall ./3rdparty/lib/libraylib.a -I. -I./3rdparty/include -L. -L./3rdparty/lib -s USE_GLFW=3 -DPLATFORM_WEB

REM asyncify
rem emcc -o index.html main.cpp -Os -Wall ./3rdparty/lib/libraylib.a -I. -I./3rdparty/include -L. -L./3rdparty/lib -s USE_GLFW=3 -s MODULARIZE=1 -s EXPORT_NAME="'MyModule'" -s EXPORTED_RUNTIME_METHODS=ccall --shell-file minshell.html -DPLATFORM_WEB

rem emcc -o game.html game.c -Os -Wall ./path-to/libraylib.a -I. -Ipath-to-raylib-h -L. -Lpath-to-libraylib-a -s USE_GLFW=3 -s ASYNCIFY --shell-file path-to/shell.html -DPLATFORM_WEB
