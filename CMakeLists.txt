cmake_minimum_required(VERSION 3.25...4.2 FATAL_ERROR)

project(
  unarr
  VERSION 1.2.0
  DESCRIPTION "A decompression library for rar, tar, 7z and zip files."
  LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CheckIPOSupported)

# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
  message(
    FATAL_ERROR
      "You cannot build in a source directory (or any \
        directory with a CMakeLists.txt file). Please make a build subdirectory. \
        Feel free to remove CMakeCache.txt and CMakeFiles.
        ")
endif()

# Build options
option(ENABLE_7Z "Build with 7z support" ON)
option(USE_SYSTEM_BZ2 "Build with system bzip2 if possible" ON)
option(USE_SYSTEM_LZMA "Build with system lzma/xz if possible" ON)
option(USE_SYSTEM_ZLIB "Build with system zlib if possible" ON)
option(USE_ZLIB_CRC "Use zlib crc32" OFF)

option(BUILD_SHARED_LIBS "Build ${PROJECT_NAME} as a shared library" ON)

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_INTEGRATION_TESTS
         "Build unarr-test executable and integration tests" OFF)
  option(BUILD_FUZZER "Build libFuzzer coverage-guided fuzzer test" OFF)
  option(BUILD_UNIT_TESTS "Build unit tests (requires CMocka)" OFF)
endif()

# Set default build type to "Release" if empty, RelWithDebInfo for fuzzing
if(PROJECT_IS_TOP_LEVEL AND BUILD_FUZZER)
  set(default_build_type "RelWithDebInfo")
else()
  set(default_build_type "Release")
endif()
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS
      "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE
      "${default_build_type}"
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

if(PROJECT_IS_TOP_LEVEL AND BUILD_FUZZER)
  if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang|AppleClang")
    add_library(fuzzer_flags INTERFACE)
    target_compile_options(
      fuzzer_flags INTERFACE -fno-omit-frame-pointer -g -O1
                             -fsanitize=fuzzer,address,undefined)
    target_link_options(fuzzer_flags INTERFACE
                        -fsanitize=fuzzer,address,undefined)
    target_compile_definitions(
      fuzzer_flags INTERFACE FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
  else()
    message(FATAL_ERROR "Fuzzer build requires a Clang compiler")
  endif()
endif()

# Build target
add_library(
  unarr
  _7z/_7z.h
  _7z/_7z.c
  common/allocator.h
  common/unarr-imp.h
  common/conv.c
  common/crc32.c
  # common/custalloc.c
  common/stream.c
  common/unarr.c
  lzmasdk/7zTypes.h
  lzmasdk/Compiler.h
  lzmasdk/CpuArch.h
  lzmasdk/Ppmd.h
  lzmasdk/Ppmd7.h
  lzmasdk/Ppmd8.h
  lzmasdk/Precomp.h
  lzmasdk/CpuArch.c
  lzmasdk/Ppmd7.c
  lzmasdk/Ppmd8.c
  lzmasdk/Ppmd7Dec.c
  lzmasdk/Ppmd7aDec.c
  lzmasdk/Ppmd8Dec.c
  rar/lzss.h
  rar/rar.h
  rar/rarvm.h
  rar/filter-rar.c
  rar/uncompress-rar.c
  rar/huffman-rar.c
  rar/rar.c
  rar/rarvm.c
  rar/parse-rar.c
  tar/tar.h
  tar/parse-tar.c
  tar/tar.c
  zip/inflate.h
  zip/zip.h
  zip/inflate.c
  zip/parse-zip.c
  zip/uncompress-zip.c
  zip/zip.c)

# Set version in header
configure_file(unarr.h.in unarr.h @ONLY)

# For CMocka unit tests
if(PROJECT_IS_TOP_LEVEL AND BUILD_UNIT_TESTS)
  add_library(unarr_h INTERFACE)
  target_include_directories(unarr_h INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
endif()

target_sources(
  unarr
  PUBLIC FILE_SET
         unarr_header
         TYPE
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/unarr.h)

target_include_directories(
  unarr PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
               $<INSTALL_INTERFACE:include>)
target_sources(unarr PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/unarr.h)

set_target_properties(
  unarr
  PROPERTIES C_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN ON
             C_EXTENSIONS OFF
             C_STANDARD_REQUIRED ON
             VERSION ${PROJECT_VERSION}
             SOVERSION ${PROJECT_VERSION_MAJOR})

target_compile_features(unarr PRIVATE c_std_99)
target_compile_definitions(
  unarr
  PRIVATE UNARR_EXPORT_SYMBOLS
  PUBLIC $<$<BOOL:${BUILD_SHARED_LIBS}>:UNARR_IS_SHARED_LIBRARY>)

# Enable lto optimization if supported
check_ipo_supported(
  RESULT lto_supported
  OUTPUT lto_diagnostic
  LANGUAGES C)
if(lto_supported)
  # Lto is discouraged for fuzzing
  if(PROJECT_IS_TOP_LEVEL AND BUILD_FUZZER)
    set_target_properties(unarr PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)
  else()
    set_target_properties(unarr PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
else()
  message(WARNING "IPO is not supported: ${lto_diagnostic}")
endif()

if(USE_SYSTEM_BZ2)
  find_package(BZip2)
endif()
if(BZIP2_FOUND)
  target_link_libraries(unarr PRIVATE BZip2::BZip2)
  target_compile_definitions(unarr PRIVATE HAVE_BZIP2)

  find_package(PkgConfig)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_BZIP2 QUIET bzip2)
  endif()
  # BZip2 upstream does not provide a .pc file, but many downstream packagers
  # do. Prefer PkgConfig if available, otherwise fall back to manual detection.
  if(PC_BZIP2_FOUND)
    list(APPEND PROJECT_REQUIRES_PRIVATE "bzip2")
  else()
    # For .pc file generation: extract library path.
    get_filename_component(BZIP2_LIB_DIR "${BZIP2_LIBRARIES}" DIRECTORY)
    if(BZIP2_LIB_DIR AND NOT BZIP2_LIB_DIR STREQUAL
                         "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
      list(APPEND PROJECT_LIBS_PRIVATE "-L${BZIP2_LIB_DIR}")
    endif()
    list(APPEND PROJECT_LIBS_PRIVATE "-lbz2")
  endif()
endif()

if(USE_SYSTEM_LZMA)
  find_package(LibLZMA)
endif()
if(LIBLZMA_FOUND)
  target_link_libraries(unarr PRIVATE LibLZMA::LibLZMA)
  target_compile_definitions(unarr PRIVATE HAVE_LIBLZMA)
  list(APPEND PROJECT_REQUIRES_PRIVATE "liblzma")
else()
  target_sources(unarr PRIVATE lzmasdk/LzmaDec.h lzmasdk/LzmaDec.c)
endif()

if(USE_SYSTEM_ZLIB)
  find_package(ZLIB)
endif()
if(ZLIB_FOUND)
  target_link_libraries(unarr PRIVATE ZLIB::ZLIB)
  target_compile_definitions(
    unarr PRIVATE HAVE_ZLIB $<$<BOOL:${USE_ZLIB_CRC}>:USE_ZLIB_CRC>)
  # Add zlib to libunarr.pc Requires.private
  list(APPEND PROJECT_REQUIRES_PRIVATE "zlib")
endif()

if(ENABLE_7Z)
  target_sources(
    unarr
    PRIVATE lzmasdk/7z.h
            lzmasdk/7zArcIn.c
            lzmasdk/7zBuf.h
            lzmasdk/7zBuf.c
            lzmasdk/7zCrc.h
            lzmasdk/7zDec.c
            lzmasdk/7zStream.c
            lzmasdk/7zWindows.h
            lzmasdk/Bcj2.h
            lzmasdk/Bcj2.c
            lzmasdk/Bra.c
            lzmasdk/Bra.h
            lzmasdk/Bra86.c
            lzmasdk/Delta.h
            lzmasdk/Delta.c
            lzmasdk/RotateDefs.h
            lzmasdk/Lzma2Dec.h
            lzmasdk/Lzma2Dec.c)
  if(LIBLZMA_FOUND) # TODO: Replace 7z lzma with system lzma
    target_sources(unarr PRIVATE lzmasdk/LzmaDec.h lzmasdk/LzmaDec.c)
  endif()
  target_compile_definitions(unarr PRIVATE HAVE_7Z Z7_PPMD_SUPPORT)
endif()

# Compiler specific settings

if(UNIX
   OR MINGW
   OR MSYS)
  target_compile_options(
    unarr
    PRIVATE -Wall
            -Wextra
            -pedantic
            -Wstrict-prototypes
            -Wmissing-prototypes
            -Werror-implicit-function-declaration
            $<$<CONFIG:Release>:-fomit-frame-pointer>
            $<$<C_COMPILER_ID:Clang,AppleClang>:
            -Wno-missing-field-initializers>)
  if(PROJECT_IS_TOP_LEVEL AND BUILD_FUZZER)
    target_link_libraries(unarr PRIVATE fuzzer_flags)
  endif()

  target_compile_definitions(unarr PRIVATE _FILE_OFFSET_BITS=64)

  # Linker flags
  if(NOT (PROJECT_IS_TOP_LEVEL AND BUILD_FUZZER))
    if("${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
      target_link_options(unarr PRIVATE "-Wl,-undefined,error")
    else()
      target_link_options(unarr PRIVATE "-Wl,--as-needed" "-Wl,--no-undefined")
    endif()
  endif()

endif()

if(MSVC)
  target_compile_options(unarr PRIVATE /W3 $<$<CONFIG:Release>:/Ox>)
  target_compile_definitions(unarr PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Alias target for unarr::unarr
add_library(unarr::unarr ALIAS unarr)

# Include tests

if(PROJECT_IS_TOP_LEVEL
   AND (BUILD_UNIT_TESTS
        OR BUILD_INTEGRATION_TESTS
        OR BUILD_FUZZER))
  enable_testing()
  add_subdirectory(test)
endif()

# Generate paths for pkg-config file
if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
  set(PROJECT_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
else()
  set(PROJECT_INSTALL_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
endif()
if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
  set(PROJECT_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
else()
  set(PROJECT_INSTALL_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
endif()

# Write pkg-config file
list(JOIN PROJECT_REQUIRES_PRIVATE " " PROJECT_REQUIRES_PRIVATE)
list(JOIN PROJECT_LIBS_PRIVATE " " PROJECT_LIBS_PRIVATE)
string(STRIP "${PROJECT_REQUIRES_PRIVATE}" PROJECT_REQUIRES_PRIVATE)
string(STRIP "${PROJECT_LIBS_PRIVATE}" PROJECT_LIBS_PRIVATE)
configure_file("pkg-config.pc.in" "lib${PROJECT_NAME}.pc" @ONLY)

# Install library and header
install(
  TARGETS unarr
  EXPORT unarr-targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          FILE_SET unarr_header
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install pkg-config file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Export and install targets
install(
  EXPORT unarr-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unarr
  NAMESPACE unarr::)

# Make project importable from build dir
export(
  TARGETS unarr
  FILE unarr-targets.cmake
  NAMESPACE unarr::)

# Write a config file for installation
configure_package_config_file(
  unarr-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/unarr-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unarr
  NO_SET_AND_CHECK_MACRO NO_CHECK_REQUIRED_COMPONENTS_MACRO)

# Write version file
write_basic_package_version_file(unarr-version.cmake
                                 COMPATIBILITY AnyNewerVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/unarr-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/unarr-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unarr)
