file(GLOB HEADERS *.h)
file(GLOB SOURCE_FILES *.cpp)

if (WIN32)
  list(APPEND SOURCE_FILES win32/emoji_win32.cpp)
  set(WIN_LIBS d2d1 dwrite Windowscodecs)
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
  list(APPEND SOURCE_FILES macos/emoji_macos.cpp)
  list(APPEND PLATFORM_FILES ios/windowless_context.mm)
elseif (APPLE)
  list(APPEND SOURCE_FILES macos/emoji_macos.cpp)
  list(APPEND PLATFORM_FILES macos/windowless_context.mm)
elseif (UNIX)
  list(APPEND SOURCE_FILES linux/emoji_linux.cpp)
endif ()

if (VISAGE_AMALGAMATED_BUILD)
  amalgamate_sources(AMALGAMATED ${CMAKE_CURRENT_BINARY_DIR}/visage_graphics_amalgamated "${SOURCE_FILES}")
  list(APPEND SOURCE_FILES ${AMALGAMATED})
endif ()

amalgamate_headers(${VISAGE_INCLUDE}/visage/graphics.h "${HEADERS}")
list(APPEND HEADERS ${VISAGE_INCLUDE}/visage/graphics.h)

add_library(VisageGraphics OBJECT ${SOURCE_FILES} ${PLATFORM_FILES} ${HEADERS})

if (APPLE)
  find_library(METAL_LIBRARY Metal)
  find_library(METALKIT_LIBRARY MetalKit)
  set(METAL_LIBS ${METAL_LIBRARY} ${METALKIT_LIBRARY})
elseif (UNIX)
  add_definitions(-D_LARGEFILE64_SOURCE)
endif ()

set(BGFX_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(BGFX_BUILD_EXAMPLE_COMMON OFF CACHE INTERNAL "" FORCE)
set(BGFX_BUILD_TOOLS OFF CACHE INTERNAL "" FORCE)
set(BGFX_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(BGFX_INSTALL OFF CACHE INTERNAL "" FORCE)
set(BX_AMALGAMATED ON CACHE INTERNAL "" FORCE)
set(BX_CONFIG_DEBUG OFF CACHE INTERNAL "" FORCE)
set(BGFX_CONFIG_DEBUG_ANNOTATION ${VISAGE_ENABLE_GRAPHICS_DEBUG_LOGGING} CACHE INTERNAL "" FORCE)
set(BGFX_AMALGAMATED ON CACHE INTERNAL "" FORCE)
set(BGFX_CONFIG_MAX_FRAME_BUFFERS 1024 CACHE INTERNAL "" FORCE)
set(BGFX_CONFIG_MAX_VIEWS 256 CACHE INTERNAL "" FORCE)
set(BGFX_CONFIG_MAX_DRAW_CALLS 8192 CACHE INTERNAL "" FORCE)
set(BGFX_WITH_WAYLAND OFF CACHE INTERNAL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)

if (VISAGE_ENABLE_GRAPHICS_DEBUG_LOGGING)
  target_compile_definitions(VisageGraphics PUBLIC VISAGE_GRAPHICS_DEBUG_LOGGING=1)
endif ()

set(BGFX_CONFIG_MULTITHREADED 0 CACHE INTERNAL "" FORCE)
if (NOT EMSCRIPTEN AND VISAGE_ENABLE_BACKGROUND_GRAPHICS_THREAD)
  set(BGFX_CONFIG_MULTITHREADED 1 CACHE INTERNAL "" FORCE)
  target_compile_definitions(VisageGraphics PUBLIC VISAGE_BACKGROUND_GRAPHICS_THREAD=1)
endif ()

message(STATUS "VISAGE: Downloading graphics dependencies")
set(CMAKE_MESSAGE_LOG_LEVEL "ERROR")
FetchContent_Declare(bgfx GIT_REPOSITORY https://github.com/bkaradzic/bgfx.cmake.git
  GIT_SHALLOW TRUE
  GIT_TAG v1.129.8958-499
  EXCLUDE_FROM_ALL
)
# bgfx.cmake declares cmake_minimum_required(VERSION 3.10.2), which forces policy CMP0091 to
# OLD for its subtree. Under OLD, CMake ignores the project-wide CMAKE_MSVC_RUNTIME_LIBRARY
# (static /MT) and takes the runtime library from the default /MD flags in CMAKE_<LANG>_FLAGS,
# so bgfx/bx/bimg compile against the dynamic CRT. A plugin that mixes our /MT code with /MD
# bgfx links without error but gains a hidden dependency on the VC++ redistributable and fails
# to load on machines that lack it ("Could not be loaded" in the host). Temporarily swap
# /MD -> /MT in the default C and C++ flags around the bgfx build, mirroring the AAX SDK
# workaround in iPlug2/Scripts/cmake/AAX.cmake. Restored afterward so later targets (freetype,
# VisageGraphics) keep selecting the runtime via CMAKE_MSVC_RUNTIME_LIBRARY.
if (MSVC)
  set(_saved_c_flags_release "${CMAKE_C_FLAGS_RELEASE}")
  set(_saved_c_flags_debug "${CMAKE_C_FLAGS_DEBUG}")
  set(_saved_c_flags_relwithdebinfo "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
  set(_saved_c_flags_minsizerel "${CMAKE_C_FLAGS_MINSIZEREL}")
  set(_saved_cxx_flags_release "${CMAKE_CXX_FLAGS_RELEASE}")
  set(_saved_cxx_flags_debug "${CMAKE_CXX_FLAGS_DEBUG}")
  set(_saved_cxx_flags_relwithdebinfo "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  set(_saved_cxx_flags_minsizerel "${CMAKE_CXX_FLAGS_MINSIZEREL}")
  string(REPLACE "/MDd" "/MTd" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
  string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
  string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
  string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")
  string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
endif ()

FetchContent_MakeAvailable(bgfx)

if (MSVC)
  set(CMAKE_C_FLAGS_RELEASE "${_saved_c_flags_release}")
  set(CMAKE_C_FLAGS_DEBUG "${_saved_c_flags_debug}")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "${_saved_c_flags_relwithdebinfo}")
  set(CMAKE_C_FLAGS_MINSIZEREL "${_saved_c_flags_minsizerel}")
  set(CMAKE_CXX_FLAGS_RELEASE "${_saved_cxx_flags_release}")
  set(CMAKE_CXX_FLAGS_DEBUG "${_saved_cxx_flags_debug}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${_saved_cxx_flags_relwithdebinfo}")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${_saved_cxx_flags_minsizerel}")
endif ()

# Belt-and-suspenders: also set the runtime-library property on the bgfx family in case a future
# bgfx.cmake bumps its cmake_minimum_required to >= 3.15 (CMP0091 NEW), where this is what counts.
if (MSVC)
  set_target_properties(bgfx bx bimg bimg_decode bimg_encode PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

FetchContent_Declare(freetype GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
  GIT_SHALLOW TRUE
  GIT_TAG VER-2-14-1
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(freetype)
set(CMAKE_MESSAGE_LOG_LEVEL "NOTICE")

if (MSVC)
  set(DISABLE_WARNINGS /W0)
else ()
  set(DISABLE_WARNINGS -w)
endif ()

target_compile_options(freetype PRIVATE ${DISABLE_WARNINGS})
target_compile_options(bgfx PRIVATE ${DISABLE_WARNINGS})
target_compile_options(bx PRIVATE ${DISABLE_WARNINGS})
target_compile_definitions(bx PRIVATE BX_CONFIG_EXCEPTION_HANDLING_USE_WINDOWS_SEH=0)
target_compile_options(bimg_decode PRIVATE ${DISABLE_WARNINGS})
target_compile_options(bimg_encode PRIVATE ${DISABLE_WARNINGS})
target_compile_options(bimg PRIVATE ${DISABLE_WARNINGS})

target_link_libraries(
  VisageGraphics
  PRIVATE
  bx
  bgfx
  bx
  bimg_decode
  bimg_encode
  freetype
  ${WIN_LIBS}
  ${METAL_LIBS}
  ${FONTCONFIG_LIBRARIES}
)

include(embedded.cmake)

target_include_directories(VisageGraphics
  PRIVATE
  ${VISAGE_INCLUDE_PATH}
  ${freetype_SOURCE_DIR}/include
  ${BGFX_DIR}/include
  ${BGFX_DIR}/3rdparty
  ${BX_DIR}/include
  ${BIMG_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(WIN32)
  target_compile_definitions(VisageGraphics PRIVATE
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
    NTDDI_VERSION=0x0A000000
  )
endif()

target_include_directories(VisageGraphics
  SYSTEM PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/3rdparty
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/tools
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bx/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/bimg/include
)

set_target_properties(VisageGraphics PROPERTIES FOLDER "visage")
set_target_properties(bgfx PROPERTIES FOLDER "visage/third_party/bgfx")
set_target_properties(bx PROPERTIES FOLDER "visage/third_party/bgfx")
set_target_properties(bimg PROPERTIES FOLDER "visage/third_party/bgfx")
set_target_properties(bimg_decode PROPERTIES FOLDER "visage/third_party/bgfx")
set_target_properties(bimg_encode PROPERTIES FOLDER "visage/third_party/bgfx")

visage_add_test_target(
  TARGET VisageGraphicsTests
  TEST_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)
set_target_properties(freetype PROPERTIES FOLDER "visage/third_party")

if (APPLE)
  target_compile_options(VisageGraphics PRIVATE -fobjc-arc)
endif ()
