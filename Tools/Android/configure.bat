set buildDir=..\..\build
set AndroidNDK=<value>
set Ninja=<value>
set abi=<value>
set buildType=Debug

del /f /q %buildDir%\CMakeCache.txt
cmake -DSRK_ENABLE_TESTS=OFF -DSRK_ENABLE_EXTERNAL_ZSTD=OFF -DSRK_ENABLE_FRAMEWORK=OFF -DCMAKE_INSTALL_PREFIX=install -DCMAKE_CXX_FLAGS="-D__cpp_lib_remove_cvref -D__cpp_lib_bitops -DSRK_std_convertible_to -DSRK_std_default_initializable -DSRK_std_derived_from -DSRK_std_floating_point -DSRK_std_integral -DSRK_std_invocable -DSRK_std_signed_integral -DSRK_std_unsigned_integral -UANDROID -llog" -DCMAKE_TOOLCHAIN_FILE=%AndroidNDK%/build/cmake/android.toolchain.cmake -DANDROID-ABI=%abi% -DANDROID_NDK=%AndroidNDK% -DANDROID_PLATFORM=android-22 -DCMAKE_BUILD_TYPE=%buildType% -DCMAKE_MAKE_PROGRAM=%Ninja% -B %buildDir% ../.. -G Ninja
pause