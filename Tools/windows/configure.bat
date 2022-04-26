del /f /q .\CMakeCache.txt
cmake -DAE_TESTS=ON -DCMAKE_INSTALL_PREFIX="install" ..
pause