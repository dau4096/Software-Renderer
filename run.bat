@ECHO off

CD src
REM Delete all .o files and force recompile.
IF EXIST "graphics.o" (
    DEL /Q "graphics.o"
)
IF EXIST "utils.o" (
    DEL /Q "utils.o"
)

CD ..
IF EXIST "main.o" (
    DEL /Q "main.o"
)

REM Compile.
mingw32-make
REM Wait for user, then start prgm.exe.
PAUSE
start prgm.exe