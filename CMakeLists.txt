cmake_minimum_required(VERSION 3.17)

project(gengine VERSION 0.5.1
                DESCRIPTION "Gabriel Knight 3 game engine"
                LANGUAGES C CXX)

set(PUBLISHER_NAME "Sierra On-Line")
set(APPLICATION_NAME "Gabriel Knight 3")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} PLAT_SOURCE_DIR)
file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR} PLAT_BINARY_DIR)

set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

include(CheckIncludeFile)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_STAT_H)
CHECK_INCLUDE_FILE(dirent.h HAVE_DIRENT_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
configure_file("${PROJECT_SOURCE_DIR}/Source/Engine/Platform/BuildEnv.h.in" "${CMAKE_CURRENT_BINARY_DIR}/BuildEnv.h")

file(GLOB_RECURSE source_files CONFIGURE_DEPENDS
    "Source/*.cpp" "Source/*.h" "Source/*.cc" "Source/*.hh"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${source_files})

file(GLOB_RECURSE asset_files CONFIGURE_DEPENDS
    "Assets/*.txt" "Assets/*.shp" "Assets/*.glsl"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${asset_files})

add_executable(gk3 ${source_files} ${asset_files})

file(GLOB_RECURSE ALL_SOURCE_FILES_AND_DIRS CONFIGURE_DEPENDS
    LIST_DIRECTORIES true
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*"
)
set(ALL_SOURCE_DIRS "")
foreach(ITEM IN LISTS ALL_SOURCE_FILES_AND_DIRS)
    if(IS_DIRECTORY "${ITEM}")
        list(APPEND ALL_SOURCE_DIRS "${ITEM}")
    endif()
endforeach()

target_include_directories(gk3 PRIVATE
    Source
    ${ALL_SOURCE_DIRS}
    Libraries/Flex/include
    Libraries/minilzo
    Libraries/stb
    Libraries/imgui
    Libraries/imgui/backends
    "${CMAKE_CURRENT_BINARY_DIR}"
)

target_compile_definitions(gk3 PRIVATE $<$<CONFIG:Debug>:DEBUG>)
add_compile_definitions(NOMINMAX)

if(MSVC)
    target_compile_options(gk3 PRIVATE /Wall
        /wd4005 /wd4061 /wd4065 /wd4100 /wd4127 /wd4191 /wd4242 /wd4244 /wd4263 /wd4264
        /wd4296 /wd4355 /wd4365 /wd4018 /wd4389 /wd4245 /wd4505 /wd4514 /wd4611 /wd4625
        /wd4626 /wd4668 /wd4710 /wd4711 /wd4774 /wd4820 /wd4866 /wd5026 /wd5027 /wd5039
        /wd5045 /wd5219 /wd5245 /wd5262
    )
    target_compile_definitions(gk3 PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(gk3 PRIVATE /fp:fast /JMC /MP)
else()
    target_compile_options(gk3 PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra
            -Wno-comment
            -Wno-inconsistent-missing-override
            -Wno-invalid-offsetof
            -Wno-overloaded-virtual
            -Wno-sign-compare
            -Wno-unused-but-set-variable
            -Wno-unused-function
            -Wno-unused-parameter
            -Wno-unused-private-field>
    )
endif()

if(WIN32)
    set(LIBS_SRC "${PLAT_SOURCE_DIR}/Libraries")

    if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
        set(WIN_ARCH "x86")
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
        set(WIN_ARCH "x64")
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        set(WIN_ARCH "arm64")
    else()
        message(FATAL_ERROR "Unsupported Windows platform: ${CMAKE_GENERATOR_PLATFORM}")
    endif()
    message(STATUS "Windows architecture: ${WIN_ARCH}")

    target_include_directories(gk3 PRIVATE
        Libraries/libpng/win/include
        Libraries/SDL/win/include
        Libraries/zlib/win/include
        Libraries/ffmpeg/include
        Libraries/GLEW/include
        Libraries/fmod/inc
    )

    target_link_directories(gk3 PRIVATE
        Libraries/ffmpeg/lib/win
        Libraries/fmod/lib/win/${WIN_ARCH}
        Libraries/GLEW/lib/win/${WIN_ARCH}
        Libraries/libpng/win/lib
        Libraries/SDL/win/lib/${WIN_ARCH}
        Libraries/zlib/win/lib
    )

    target_link_libraries(gk3
        avcodec avformat avutil swresample swscale
        fmod_vc
        glew32
        libpng16
        zdll
        SDL2 SDL2main
        opengl32
    )

    set(LIB_DEST "${PLAT_BINARY_DIR}/$<CONFIG>")
    set(LIB_FFMPEG_SRC "${LIBS_SRC}/ffmpeg/lib/win")

    add_custom_command(TARGET gk3
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/avcodec-58.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/avformat-58.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/avutil-56.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/swresample-3.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIB_FFMPEG_SRC}/swscale-5.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/fmod/lib/win/${WIN_ARCH}/fmod.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/GLEW/lib/win/${WIN_ARCH}/glew32.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/libpng/win/lib/libpng16.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/SDL/win/lib/${WIN_ARCH}/SDL2.dll" "${LIB_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy "${LIBS_SRC}/zlib/win/lib/zlib1.dll" "${LIB_DEST}"
        VERBATIM
    )
else()
    message(STATUS "Using system libraries for ${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_PROCESSOR})")

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(GLEW REQUIRED glew)
    pkg_check_modules(PNG REQUIRED libpng)
    pkg_check_modules(ZLIB REQUIRED zlib)

    target_include_directories(gk3 PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${PNG_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIRS}
    )

    target_link_directories(gk3 PRIVATE
        ${SDL2_LIBRARY_DIRS}
        ${GLEW_LIBRARY_DIRS}
        ${PNG_LIBRARY_DIRS}
        ${ZLIB_LIBRARY_DIRS}
    )

    target_link_libraries(gk3
        ${SDL2_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${PNG_LIBRARIES}
        ${ZLIB_LIBRARIES}
    )

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        message(STATUS "Using static FFmpeg 4 for Linux")

        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            set(FFMPEG_ARCH "x86_64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
            set(FFMPEG_ARCH "aarch64")
        else()
            set(FFMPEG_ARCH "x86")
        endif()

        add_compile_definitions(__STDC_CONSTANT_MACROS)
        add_compile_definitions(__STDC_LIMIT_MACROS)
        add_compile_definitions(__STDC_FORMAT_MACROS)

        set(FFMPEG_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/ffmpeg/build/${FFMPEG_ARCH}/install/lib")
        set(FFMPEG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/ffmpeg/build/${FFMPEG_ARCH}/install/include")

        target_include_directories(gk3 PRIVATE ${FFMPEG_INCLUDE_DIR})
        target_link_libraries(gk3
            ${FFMPEG_LIB_DIR}/libavcodec.a
            ${FFMPEG_LIB_DIR}/libavformat.a
            ${FFMPEG_LIB_DIR}/libavutil.a
            ${FFMPEG_LIB_DIR}/libswresample.a
            ${FFMPEG_LIB_DIR}/libswscale.a
        )

        find_package(ZLIB REQUIRED)
        target_link_libraries(gk3 ${ZLIB_LIBRARIES} bz2 dl xcb m)

    else()
        message(STATUS "Using dynamic FFmpeg for macOS")
        pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswresample libswscale)
        target_include_directories(gk3 PRIVATE ${FFMPEG_INCLUDE_DIRS})
        target_link_directories(gk3 PRIVATE ${FFMPEG_LIBRARY_DIRS})
        target_link_libraries(gk3 ${FFMPEG_LIBRARIES})
    endif()

    target_include_directories(gk3 PRIVATE Libraries/fmod/inc)

    if(APPLE)
        target_link_directories(gk3 PRIVATE Libraries/fmod/lib/mac)
        target_link_libraries(gk3 fmod)
    else()
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            set(FMOD_ARCH "x86_64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
            set(FMOD_ARCH "aarch64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
            set(FMOD_ARCH "armhf")
        else()
            set(FMOD_ARCH "x86")
        endif()
        message(STATUS "FMOD architecture: ${FMOD_ARCH}")
        target_link_directories(gk3 PRIVATE Libraries/fmod/lib/linux/${FMOD_ARCH})
        target_link_libraries(gk3 fmod)
    endif()

    if(APPLE)
        find_library(OPENGL_LIB OpenGL)
        find_library(COREFOUNDATION CoreFoundation)
        target_link_libraries(gk3 ${OPENGL_LIB} ${COREFOUNDATION})
    else()
        target_link_libraries(gk3 GL)
        find_library(DL_LIB dl)
        find_library(PTHREAD_LIB pthread)
        target_link_libraries(gk3 ${DL_LIB} ${PTHREAD_LIB})
    endif()

    if(APPLE)
        set(MAC_APP_CONTENTS "${PLAT_BINARY_DIR}/gk3.app/Contents")
        if(XCODE)
            set(MAC_APP_CONTENTS "${PLAT_BINARY_DIR}/$<CONFIG>/gk3.app/Contents")
        endif()
        set(LIB_DEST "${MAC_APP_CONTENTS}/Libraries")
        add_custom_command(TARGET gk3
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${LIB_DEST}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/fmod/lib/mac/libfmod.dylib" "${LIB_DEST}"
            VERBATIM
        )
    else()
        set(LIB_DEST "${PLAT_BINARY_DIR}")
        add_custom_command(TARGET gk3
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/fmod/lib/linux/${FMOD_ARCH}/libfmod.so" "${LIB_DEST}"
            COMMAND ${CMAKE_COMMAND} -E create_symlink "${PLAT_SOURCE_DIR}/Assets" "${LIB_DEST}/Assets"
            COMMAND ${CMAKE_COMMAND} -E create_symlink "${PLAT_SOURCE_DIR}/Data" "${LIB_DEST}/Data"
            COMMAND ${CMAKE_COMMAND} -E create_symlink "${PLAT_SOURCE_DIR}/GK3.ini" "${LIB_DEST}/GK3.ini"
            VERBATIM
        )
    endif()
endif()

set(LZO_SOURCES
    Libraries/minilzo/lzoconf.h
    Libraries/minilzo/lzodefs.h
    Libraries/minilzo/minilzo.c
    Libraries/minilzo/minilzo.h
)
target_sources(gk3 PRIVATE ${LZO_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${LZO_SOURCES})

set(STB_SOURCES
    Libraries/stb/stb_image_resize.h
    Libraries/stb/stb_image_resize.cpp
)
target_sources(gk3 PRIVATE ${STB_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${STB_SOURCES})

set(IMGUI_SOURCES
    Libraries/imgui/imgui.cpp
    Libraries/imgui/imgui_demo.cpp
    Libraries/imgui/imgui_draw.cpp
    Libraries/imgui/imgui_stdlib.cpp
    Libraries/imgui/imgui_tables.cpp
    Libraries/imgui/imgui_widgets.cpp
    Libraries/imgui/backends/imgui_impl_opengl3.cpp
    Libraries/imgui/backends/imgui_impl_sdl.cpp
)
target_sources(gk3 PRIVATE ${IMGUI_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${IMGUI_SOURCES})

if(WIN32)
    set_property(TARGET gk3 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT gk3)
    set(APP_RESOURCES Publish/Windows/App.rc)
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${APP_RESOURCES})
    target_sources(gk3 PRIVATE ${APP_RESOURCES})
elseif(APPLE)
    set_target_properties(gk3 PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Publish/Mac/Info.plist
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.kromenak.gengine"
        XCODE_ATTRIBUTE_MARKETING_VERSION ${PROJECT_VERSION}
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION 1
    )
    target_sources(gk3 PRIVATE Publish/Mac/Assets.xcassets)
    set_source_files_properties(Publish/Mac/Assets.xcassets PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

add_custom_target(deploy DEPENDS "deploy_output")
add_dependencies(deploy gk3)

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(DEPLOY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Bin/$<CONFIG>")
if(CMAKE_CONFIGURATION_TYPES)
    set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
else()
    set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif()

add_custom_command(
    OUTPUT "deploy_output_clean"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${DEPLOY_DIR}"
    VERBATIM
)

if(WIN32)
    set(ZIP_FILE_NAME "GK3-Win-${PROJECT_VERSION}.zip")
    set(ASSETS_ROOT_DIR "${DEPLOY_DIR}")
    add_custom_command(
        OUTPUT "deploy_output_exe"
        DEPENDS "deploy_output_clean"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${BUILD_DIR}" "${DEPLOY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${DEPLOY_DIR}/gk3.pdb"
        COMMAND ${CMAKE_COMMAND} -E rename "${DEPLOY_DIR}/gk3.exe" "${DEPLOY_DIR}/GK3.exe"
        VERBATIM
    )
elseif(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(ZIP_FILE_NAME "GK3-Mac-${PROJECT_VERSION}-arm64.zip")
    else()
        set(ZIP_FILE_NAME "GK3-Mac-${PROJECT_VERSION}-x86_64.zip")
    endif()
    set(APP_FILENAME "${APPLICATION_NAME}.app")
    set(ASSETS_ROOT_DIR "${DEPLOY_DIR}/${APP_FILENAME}/Contents/Resources")
    add_custom_command(
        OUTPUT "deploy_output_exe"
        DEPENDS "deploy_output_clean"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${BUILD_DIR}/gk3.app" "${DEPLOY_DIR}/${APP_FILENAME}"
        VERBATIM
    )
else()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
        set(ARCH_SUFFIX "-arm64")
    else()
        set(ARCH_SUFFIX "")
    endif()
    set(ZIP_FILE_NAME "GK3-Linux-${PROJECT_VERSION}${ARCH_SUFFIX}.zip")
    set(APP_FILENAME "gk3")
    set(ASSETS_ROOT_DIR "${DEPLOY_DIR}")
    add_custom_command(
        OUTPUT "deploy_output_exe"
        DEPENDS "deploy_output_clean"
        COMMAND ${CMAKE_COMMAND} -E copy "${BUILD_DIR}/gk3" "${DEPLOY_DIR}/${APP_FILENAME}"
        COMMAND patchelf --set-rpath "$ORIGIN" "${DEPLOY_DIR}/${APP_FILENAME}"
        COMMAND sh -c "cp ${BUILD_DIR}/*.so \"${DEPLOY_DIR}\" 2>/dev/null || true"
        VERBATIM
    )
endif()

add_custom_command(
    OUTPUT "deploy_output_shared"
    DEPENDS "deploy_output_exe"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ROOT_DIR}/Assets" "${ASSETS_ROOT_DIR}/Assets"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${ASSETS_ROOT_DIR}/Data"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ASSETS_ROOT_DIR}/Data"
    COMMAND ${CMAKE_COMMAND} -E copy "${ROOT_DIR}/Data/README.md" "${ASSETS_ROOT_DIR}/Data"
    COMMAND ${CMAKE_COMMAND} -E copy "${ROOT_DIR}/GK3.ini" "${ASSETS_ROOT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "${ROOT_DIR}/Publish/README.md" "${DEPLOY_DIR}"
    VERBATIM
)

add_custom_command(
    OUTPUT "deploy_output"
    DEPENDS "deploy_output_shared"
    COMMAND ${CMAKE_COMMAND} -E tar cvf "${ROOT_DIR}/${ZIP_FILE_NAME}" --format=zip .
    COMMAND ${CMAKE_COMMAND} -E rename "${ROOT_DIR}/${ZIP_FILE_NAME}" "${DEPLOY_DIR}/${ZIP_FILE_NAME}"
    WORKING_DIRECTORY "${DEPLOY_DIR}"
    VERBATIM
)

add_subdirectory(Tests)
