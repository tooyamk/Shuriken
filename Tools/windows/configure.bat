del /f /q .\CMakeCache.txt
cmake -DSRK_TESTS=ON -DCMAKE_INSTALL_PREFIX="install" ..
pause