@echo off
gcc -o main src\main.c -I libs\include -L libs\lib -l:libglfw3.a -l:glew32s.lib -lopengl32 -lgdi32 -l:cglm.lib
pause