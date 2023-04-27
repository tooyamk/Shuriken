srkSetSubProject(${SRK_TOOL_PROJ_PREFIX})

if (SRK_HOST_OS_WINDOWS)
    set(OUTPUT_NAME zlibstatic.lib)
else ()
    set(OUTPUT_NAME libz.a)
endif ()

set(installDir ${PROJECT_BINARY_DIR}/install)

set_property(GLOBAL PROPERTY SRKPROJ_TOOL_ZLIB_NAME ${PROJECT_NAME})
set_property(GLOBAL PROPERTY SRKPROJ_TOOL_ZLIB_INCLUDE ${installDir}/include)
set_property(GLOBAL PROPERTY SRKPROJ_TOOL_ZLIB_LIB_DIR ${installDir}/lib)
set_property(GLOBAL PROPERTY SRKPROJ_TOOL_ZLIB_SLIB ${installDir}/lib/${OUTPUT_NAME})

set(gitTag v1.2.13)
srkCheckExternalProjectVersion_GitTag(${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src ${gitTag})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${PROJECT_BINARY_DIR}
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG ${gitTag}
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    TMP_DIR tmp
    STAMP_DIR stamp
    DOWNLOAD_DIR download
    INSTALL_DIR install
    SOURCE_DIR src
    BINARY_DIR ${SRK_PROJECT_BUILD_ROOT_DIR}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/install -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release -B ${SRK_PROJECT_BUILD_ROOT_DIR} ${PROJECT_BINARY_DIR}/src
    BUILD_COMMAND ${CMAKE_COMMAND} --build ${SRK_PROJECT_BUILD_ROOT_DIR} --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --build ${SRK_PROJECT_BUILD_ROOT_DIR} --config Release --target install
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${SRK_TOOLS_DIR_NAME})