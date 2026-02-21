@echo off

call C:\msvc\setup_x64.bat

cd %~dp0

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

set CommonLinkerFlags=-opt:ref -incremental:no user32.lib Gdi32.lib winmm.lib Opengl32.lib

set CommonCompilerFlags=-MTd -Gm- -nologo -GR- -EHa- -Oi -FC -Z7 -WX -W4 -wd4459 -wd4456 -wd4201 -wd4100 -wd4101 -wd4189 -wd4505 -wd4996 -wd4389 -wd4244 -DEDITOR__INTERNAL=1 -I..\source  /std:c++20 /Zc:strictStrings-

REM 64-Bit

echo WAITING FOR PDB > lock.tmp

if not exist editor_libs.obj (
    cl %CommonCompilerFlags% -DEDITOR_SLOW_COMPILE=1 -c /TP -Foeditor_libs.obj -Fmeditor_libs.map ..\source\editor\editor_libs.h -LD /link /DLL %CommonLinkerFlags% /EXPORT:UpdateAndRender
)

cl %CommonCompilerFlags% -DEDITOR_INTERNAL=1 -Fmeditor_app.map ..\source\editor\editor_app.cpp .\editor_libs.obj -LD /link /DLL %CommonLinkerFlags% /EXPORT:UpdateAndRender

del lock.tmp

cl %CommonCompilerFlags% -DEDITOR_INTERNAL=1 -Feeditor.exe -Fmeditor.map ..\source\editor\editor_platform.cpp /link %CommonLinkerFlags%
