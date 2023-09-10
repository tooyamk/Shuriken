execute_process(COMMAND git -C ${SRC_DIR} clean -xfd)
execute_process(COMMAND git -C ${SRC_DIR} reset --hard)

set(file ${SRC_DIR}/CMakeLists.txt)
file(READ ${file} content)
string(REGEX REPLACE "add_subdirectory\\(cmake/modules\\).+include\\(CoverageReport\\)" "add_subdirectory(cmake/modules)\r\ninstall(DIRECTORY include/dxc DESTINATION include FILES_MATCHING PATTERN \"*.h\")\r\ninclude(CoverageReport)" content "${content}")
string(REPLACE "find_package(PythonInterp 3 REQUIRED)" "" content "${content}")
string(REPLACE "dxc;" "" content "${content}")
file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/cmake/modules/AddLLVM.cmake)
file(READ ${file} content)
#string(REGEX REPLACE "set_target_properties\\(\\$\\{name}[^\r\n]*[\r\n]+[^\r\n]*PROPERTIES[^\r\n]*[\r\n]+[^\r\n]*SOVERSION[^\r\n]+[\r\n]+[^\r\n]*VERSION \\$\\{LLVM_VERSION_MAJOR}\\.\\$\\{LLVM_VERSION_MINOR}\\.\\$\\{LLVM_VERSION_PATCH}\\$\\{LLVM_VERSION_SUFFIX}\\)" "" content "${content}")
string(REGEX REPLACE "install\\(TARGETS \\$\\{name\\}[\r\n]+[^\r\n]+EXPORT LLVMExports[\r\n]+[^\r\n]+\\$\\{install_type\\} DESTINATION lib\\$\\{LLVM_LIBDIR_SUFFIX\\}[\r\n]+[^\r\n]+COMPONENT \\$\\{name\\}\\)" 
"if(\${name} STREQUAL \"LLVMDxcSupport\")\r\ninstall(TARGETS \${name} EXPORT LLVMExports \${install_type} DESTINATION lib\${LLVM_LIBDIR_SUFFIX} COMPONENT \${name})\r\nendif()" content "${content}")
file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/cmake/modules/CrossCompile.cmake)
file(READ ${file} content)
string(REPLACE "function(llvm_create_cross_target_internal target_name toochain buildtype)" "function(llvm_create_cross_target_internal target_name toolchain buildtype)" content "${content}")
string(REPLACE "add_custom_command(OUTPUT \${LLVM_\${target_name}_BUILD}/CMakeCache.txt" 
"  include(\${PREDEFINED_CONFIG_PARAMS_FILE})\r\n\  __get_predefined_config_params(predefined_params)\r\n  list(APPEND predefined_params -DDXC_BUILD_ARCH=\${DXC_BUILD_HOST_ARCH} -DPYTHON_EXECUTABLE=\${PYTHON_EXECUTABLE})\r\nadd_custom_command(OUTPUT \${LLVM_\${target_name}_BUILD}/CMakeCache.txt" content "${content}")
string(REPLACE "COMMAND \${CMAKE_COMMAND} -G \"\${CMAKE_GENERATOR}\"" "COMMAND \${CMAKE_COMMAND} \${predefined_params}" content "${content}")
string(REPLACE "-G \"\${CMAKE_GENERATOR}\" -DLLVM_TARGETS_TO_BUILD=\${LLVM_TARGETS_TO_BUILD}" "-DLLVM_TARGETS_TO_BUILD=\${LLVM_TARGETS_TO_BUILD} \${predefined_params}" content "${content}")
file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/cmake/modules/GetHostTriple.cmake)
file(READ ${file} content)
string(REPLACE "if( MSVC )" "if(CMAKE_HOST_SYSTEM_NAME MATCHES \"Windows\")" content "${content}")
string(REPLACE "( MSVC )" "()" content "${content}")
file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/cmake/modules/TableGen.cmake)
file(READ ${file} content)
string(REPLACE "if (NOT CMAKE_CONFIGURATION_TYPES)" "if ((NOT EXISTS \"\${LLVM_NATIVE_BUILD}/LLVM.sln\") AND (NOT CMAKE_CONFIGURATION_TYPES))" content "${content}")
string(REPLACE "if (\${project} STREQUAL LLVM AND NOT LLVM_INSTALL_TOOLCHAIN_ONLY)" "if (False)" content "${content}")
file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/external/SPIRV-Tools/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "find_host_package(Python3 REQUIRED)" "" content "${content}")
file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/external/SPIRV-Tools/source/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "Python3::Interpreter" "\${PYTHON_EXECUTABLE}" content "${content}")
file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/lib/DxcSupport/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "target_link_libraries(LLVMDxcSupport PUBLIC LLVMSupport)" "target_link_libraries(LLVMDxcSupport PRIVATE \$<BUILD_LOCAL_INTERFACE:LLVMSupport>)" content "${content}")
file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/lib/DxcSupport/WinFunctions.cpp)
file(READ ${file} content)
string(REPLACE "unsigned char _BitScanForward" "#ifndef __ANDROID__\r\nunsigned char _BitScanForward" content "${content}")
string(REPLACE "HANDLE CreateFile2" "#endif\r\nHANDLE CreateFile2" content "${content}")
file(WRITE ${file} "#include <ios>\n${content}")

set(file ${SRC_DIR}/tools/clang/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "if (NOT LLVM_INSTALL_TOOLCHAIN_ONLY OR \${name} STREQUAL \"libclang\")" "if (\${name} STREQUAL \"dxcompiler\")" content "${content}")
string(REPLACE "if (NOT LLVM_INSTALL_TOOLCHAIN_ONLY)" "if (False)" content "${content}")
string(REGEX REPLACE "install\\(DIRECTORY include/clang-c.+add_definitions\\( -D_GNU_SOURCE \\)" "add_definitions( -D_GNU_SOURCE )" content "${content}")
file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/tools/clang/tools/dxcompiler/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "VERSION \${LIBCLANG_LIBRARY_VERSION}" "" content "${content}")
file(WRITE ${file} "${content}")

#set(file ${SRC_DIR}/utils/hct/hctgen.py)
#file(READ ${file} content)
#string(REPLACE "def openOutput(args):" "def openOutput(args):\r\n  dir = os.path.dirname(args.output)\r\n  if not os.path.exists(dir):\r\n    os.makedirs(dir)" content "${content}")
#file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/tools/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "add_llvm_tool_subdirectory(llvm-config)" "" content "${content}")
string(REPLACE "add_llvm_tool_subdirectory(opt)" "" content "${content}")
string(REPLACE "add_llvm_tool_subdirectory(llvm-as)" "" content "${content}")
string(REPLACE "add_llvm_tool_subdirectory(llvm-dis)" "" content "${content}")
string(REPLACE "add_llvm_tool_subdirectory(dxexp)" "" content "${content}")
string(REPLACE "add_llvm_tool_subdirectory(llvm-link)" "" content "${content}")
string(REPLACE "add_llvm_tool_subdirectory(llvm-extract)" "" content "${content}")
string(REPLACE "add_llvm_tool_subdirectory(llvm-diff)" "" content "${content}")
string(REPLACE "add_llvm_tool_subdirectory(llvm-bcanalyzer)" "" content "${content}")
string(REPLACE "add_llvm_tool_subdirectory(llvm-stress)" "" content "${content}")
string(REPLACE "add_llvm_tool_subdirectory(verify-uselistorder)" "" content "${content}")
#string(REPLACE "add_llvm_external_project(clang)" "" content "${content}")
file(WRITE ${file} "${content}")

#set(file ${SRC_DIR}/tools/clang/include/CMakeLists.txt)
#file(READ ${file} content)
#string(REPLACE "add_subdirectory(clang)" "" content "${content}")
#file(WRITE ${file} "${content}")

set(file ${SRC_DIR}/tools/clang/tools/CMakeLists.txt)
file(READ ${file} content)
string(REPLACE "add_subdirectory(dxc)" "" content "${content}")
string(REPLACE "add_subdirectory(d3dcomp)" "" content "${content}")
string(REPLACE "add_subdirectory(dxrfallbackcompiler)" "" content "${content}")
string(REPLACE "add_subdirectory(dxa)" "" content "${content}")
string(REPLACE "add_subdirectory(dxopt)" "" content "${content}")
string(REPLACE "add_subdirectory(dxl)" "" content "${content}")
string(REPLACE "add_subdirectory(dxr)" "" content "${content}")
string(REPLACE "add_subdirectory(dxv)" "" content "${content}")
string(REPLACE "add_subdirectory(dxlib-sample)" "" content "${content}")
string(REPLACE "add_subdirectory(dotnetc)" "" content "${content}")
string(REPLACE "add_subdirectory(dxclib)" "" content "${content}")
file(WRITE ${file} "${content}")