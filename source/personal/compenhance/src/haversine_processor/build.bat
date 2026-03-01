@echo off

call C:\msvc\setup_x64.bat

REM Go to parent dir
cd %~dp0

IF NOT EXIST ..\..\build mkdir ..\..\build

pushd ..\..\build

set Source=..\src\json_parser\json_parser.cpp

set Flags=-fsanitize-trap -nostdinc++ -g -I../src/lib
Set Flags=%Flags% -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-sign-conversion -Wno-sign-compare -Wno-double-promotion -Wno-unused-but-set-variable -Wno-unused-variable -Wno-write-strings -Wno-pointer-arith -Wno-unused-parameter -Wno-unused-function
set Flags=%Flags% -fdiagnostics-absolute-paths -ftime-trace -Wno-null-dereference -Wno-missing-braces -Wno-vla-extension -Wno-writable-strings -Wno-missing-field-initializers -Wno-address-of-temporary -Wno-int-to-void-pointer-cast
Set Flags=%Flags% -DOS_WINDOWS=1

echo %Source%
clang %Flags% -o json_parser.exe %Source%