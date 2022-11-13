SET buildPath=%~dp0
if not exist %buildPath%\build mkdir %buildPath%\build
cd %buildPath%\build
conan install .. --build=missing -s cppstd=20 -s build_type=Release
cmake .. -G "Visual Studio 17"