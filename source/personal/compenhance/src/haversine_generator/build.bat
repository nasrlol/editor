@echo off

call C:\msvc\setup_x64.bat

REM Go to parent dir
cd %~dp0

IF NOT EXIST ..\..\build mkdir ..\..\build

pushd ..\..\build

set Source=..\src\haversine_generator\haversine_generator.cpp

set Flags=-fsanitize-trap -g -I..\src\lib
REM set Flags=-nostdinc++
set Flags=%Flags% -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-sign-conversion -Wno-sign-compare -Wno-double-promotion -Wno-unused-but-set-variable -Wno-unused-variable -Wno-write-strings -Wno-pointer-arith -Wno-unused-parameter -Wno-unused-function
set Flags=%Flags% -fdiagnostics-absolute-paths -ftime-trace -Wno-null-dereference -Wno-missing-braces -Wno-vla-extension -Wno-writable-strings -Wno-missing-field-initializers -Wno-address-of-temporary -Wno-int-to-void-pointer-cast
Set Flags=%Flags% -DOS_WINDOWS=1 -DOS_LINUX=0 

REM Flags necessary for cl
REM set Flags=%Flags% -Wno-newline-eof -Wno-keyword-macro -Wno-old-style-cast -Wno-missing-prototypes -Wno-c++98-compat-pedantic -Wno-missing-variable-declarations -Wno-zero-as-null-pointer-constant -Wno-extra-semi-stmt -Wno-unused-macros -Wno-unsafe-buffer-usage -Wno-unsafe-buffer-usage-in-libc-call
REM set Flags=%Flags% -Wno-cast-align

echo %Source%
clang %Flags% -o haversine_generator.exe %Source% "C:\Program Files\LLVM\lib\clang\21\lib\windows\clang_rt.builtins-x86_64.lib"