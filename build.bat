rem @echo off

setlocal
pushd "%~dp0"

if not defined SDL2_PATH echo SDL2_PATH not defined & exit /b 1

rem Unpack arguments
for %%a in (%*) do set "%%a=1"

rem Default arguments
if not "%release%"=="1" if not "%debug%"=="1" ^
echo Build mode not specified, defaulting to `debug` && set debug=1
if not "%msvc%"=="1" if not "%clang%"=="1" ^
echo Compiler not specified, defaulting to `msvc` && set msvc=1

rem Compile stage
set cl_common=/D_CRT_SECURE_NO_WARNINGS shell32.lib /Z7 /Fo:./obj/ ^
/I../src /I%SDL2_PATH%/include
set clang_common=-D_CRT_SECURE_NO_WARNINGS -lshell32 -g ^
-I../src -I%SDL2_PATH%/include

set cl_debug=call cl %cl_common% /DBUILD_DEBUG=1 /MDd /Od /Ob1 ^
%SDL2_PATH%/lib/SDL2maind.lib %SDL2_PATH%/lib/SDL2d.lib
set clang_debug=call clang %clang_common% -lmsvcrtd -O0 -DBUILD_DEBUG=1 ^
-l%SDL2_PATH%/lib/SDL2maind -l%SDL2_PATH%/lib/SDL2d

set cl_release=call cl %cl_common% /MD /O2 ^
%SDL2_PATH%/lib/SDL2main.lib %SDL2_PATH%/lib/SDL2.lib
set clang_release=call clang %clang_common% -lmsvcrt -O2 ^
-l%SDL2_PATH%/lib/SDL2main -l%SDL2_PATH%/lib/SDL2

rem Link stage
set cl_link=/link /subsystem:windows
set clang_link=-Xlinker /subsystem:windows -Xlinker /nodefaultlib:msvcrt ^
-Xlinker /nodefaultlib:libcmt
set cl_out=/out:
set clang_out= -o

rem Choose compile/link lines
if "%msvc%"=="1" (
  set compile_debug=%cl_debug%
  set compile_release=%cl_release%
  set compile_link=%cl_link%
  set out=%cl_out%
)

if "%clang%"=="1" (
  set compile_debug=%clang_debug%
  set compile_release=%clang_release%
  set compile_link=%clang_link%
  set out=%clang_out%
)

if "%debug%"=="1" set compile=%compile_debug%
if "%release%"=="1" set compile=%compile_release%

rem Build program
if not exist build mkdir build
pushd build
if not exist obj if "%msvc%"=="1" mkdir obj
%compile% ^
  ../src/main.c ^
  %compile_link% %out%chip8.exe || exit /b 1
popd

popd
