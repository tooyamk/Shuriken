 #!/bin/sh
cmake -DAE_TESTS=ON -DZLIB=ON -DLIBPNG=ON -DGLEW=ON -DDX_SHADER_COMPILER=ON -DSPIRV_CROSS=ON -DAE_FRAMEWORK=ON -DAE_PROGRAM_SOURCE_TRANSLATOR=ON -DAE_PNG_CONVERTER=ON -DAE_FBX_CONVERTER=ON -DAE_SHADER_SCRIPT=ON -DAE_GRAPHICS_GL=ON -DCMAKE_INSTALL_PREFIX="install" -DCMAKE_BUILD_TYPE=Debug  ..