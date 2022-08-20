set buildDir=..\..\build

del /f /q %buildDir%\CMakeCache.txt
cmake -DSRK_ENABLE_TESTS=ON -DCMAKE_INSTALL_PREFIX=install -B %buildDir% ../..
pause