@ECHO OFF

set VC_ROOT=%VS140COMNTOOLS%..\..\VC\

if exist "%VC_ROOT%vcvarsall.bat" (
    call "%VC_ROOT%vcvarsall.bat" x86_amd64
)

MKDIR build_debug
PUSHD build_debug

conan install .. --build=missing -s build_type=Debug -s compiler="Visual Studio" -s compiler.runtime="MDd"

cmake -G "Visual Studio 14 Win64" ..

POPD

MKDIR build_release
PUSHD build_release

conan install .. --build=missing -s build_type=Release -s compiler="Visual Studio" -s compiler.runtime="MD"

cmake -G "Visual Studio 14 Win64" ..

POPD
