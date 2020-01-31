cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(unarr VERSION 1.1.0 LANGUAGES C)
set(PROJECT_DESCRIPTION
  "A decompression library for rar, tar and zip files.")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Set build type to default if unset.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

# Build options
option(BUILD_SHARED_LIBS "Build ${PROJECT_NAME} as a shared library" ON)
option(BUILD_SAMPLES "Build unarr-test executable" OFF)
option(ENABLE_7Z "Build with 7z support" OFF)
option(USE_SYSTEM_BZ2 "Build with system bzip2 if possible" ON)
option(USE_SYSTEM_LZMA "Build with system lzma/xz if possible" ON)
option(USE_SYSTEM_ZLIB "Build with system zlib if possible" ON)

# Build target
add_library(unarr
  _7z/_7z.h
  _7z/_7z.c
  common/allocator.h
  common/unarr-imp.h
  common/conv.c
  common/crc32.c
  #common/custalloc.c
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

set_target_properties(unarr PROPERTIES
                      PUBLIC_HEADER unarr.h
                      C_VISIBILITY_PRESET hidden
                      C_STANDARD 99
                      C_STANDARD_REQUIRED ON
                      DEFINE_SYMBOL UNARR_EXPORT_SYMBOLS
                      VERSION ${PROJECT_VERSION}
                      SOVERSION ${PROJECT_VERSION_MAJOR})

if(BUILD_SHARED_LIBS)
  target_compile_definitions(unarr PUBLIC UNARR_IS_SHARED_LIBRARY)
endif()

if(USE_SYSTEM_BZ2)
  find_package(BZip2)
endif()
if(BZIP2_FOUND)
  target_link_libraries(unarr PRIVATE BZip2::BZip2)
  target_compile_definitions(unarr PRIVATE -DHAVE_BZIP2)
  # Bzip2 upstream does not supply a .pc file. Add it to Libs.private.
  set(PROJECT_LIBS_PRIVATE "-I${BZIP2_INCLUDE_DIRS} -l${BZIP2_LIBRARIES}")
  set(UNARR_DEPENDS_BZip2 "find_dependency(BZip2)")
endif()

if(USE_SYSTEM_LZMA)
  find_package(LibLZMA)
endif()
if(LIBLZMA_FOUND)
  target_link_libraries(unarr PRIVATE LibLZMA::LibLZMA)
  target_compile_definitions(unarr PRIVATE -DHAVE_LIBLZMA)
  set(PROJECT_REQUIRES_PRIVATE "${PROJECT_REQUIRES_PRIVATE} liblzma")
  set(UNARR_DEPENDS_LibLZMA "find_dependency(LibLZMA)")
else()
  target_sources(unarr PRIVATE
    lzmasdk/LzmaDec.h
    lzmasdk/LzmaDec.c)
endif()

if(USE_SYSTEM_ZLIB)
  find_package(ZLIB)
endif()
if(ZLIB_FOUND)
  target_link_libraries(unarr PRIVATE ZLIB::ZLIB)
  target_compile_definitions(unarr PRIVATE -DHAVE_ZLIB)
  # Add zlib to libunarr.pc Requires.private
  set(PROJECT_REQUIRES_PRIVATE "${PROJECT_REQUIRES_PRIVATE} zlib")
  set(UNARR_DEPENDS_ZLIB "find_dependency(ZLIB)")
endif()

if(ENABLE_7Z)
  target_sources(unarr PRIVATE
    lzmasdk/7z.h
    lzmasdk/7zArcIn.c
    lzmasdk/7zBuf.h
    lzmasdk/7zBuf.c
    lzmasdk/7zDec.c
    lzmasdk/7zStream.c
    lzmasdk/Bcj2.h
    lzmasdk/Bcj2.c
    lzmasdk/Bra.c
    lzmasdk/Bra.h
    lzmasdk/Bra86.c
    lzmasdk/BraIA64.c
    lzmasdk/7zCrc.h
    lzmasdk/Delta.h
    lzmasdk/Delta.c
    lzmasdk/Lzma2Dec.h
    lzmasdk/Lzma2Dec.c)
    if(LIBLZMA_FOUND)
      target_sources(unarr PRIVATE
        lzmasdk/LzmaDec.h
        lzmasdk/LzmaDec.c)
    endif()
target_compile_definitions(unarr PRIVATE -DHAVE_7Z -D_7ZIP_PPMD_SUPPPORT)
endif()

if(BUILD_SAMPLES)
  add_executable(unarr-test
                test/main.c)
  add_dependencies(unarr-test unarr)
  target_link_libraries(unarr-test unarr)
endif()

# Compiler specific settings

if(UNIX OR MINGW OR MSYS)
  target_compile_options(unarr PRIVATE -Wall -Wextra -pedantic
                    -Wstrict-prototypes -Wmissing-prototypes
                    -Werror-implicit-function-declaration
                    $<$<CONFIG:Release>:-fomit-frame-pointer>
                    $<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>>:
                    -Wno-missing-field-initializers>
                    -flto)
  target_compile_definitions(unarr PRIVATE -D_FILE_OFFSET_BITS=64)

  # Linker flags

  # Clang linker needs -flto too when doing lto
  if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set_target_properties(unarr PROPERTIES LINK_FLAGS
                          "-Wl,--no-undefined -Wl,--as-needed -flto")
  # Apple ld uses different syntax for undefined symbol check
  elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
    set_target_properties(unarr PROPERTIES LINK_FLAGS
                          "-Wl,-undefined,error -flto")
  else()
    set_target_properties(unarr PROPERTIES LINK_FLAGS
                          "-Wl,--no-undefined -Wl,--as-needed")
  endif()
endif()

if(MSVC)
  target_compile_options(unarr PRIVATE /W3
                        $<$<CONFIG:Release>:/Ox>)
  target_compile_definitions(unarr PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Write pkg-config file
configure_file("pkg-config.pc.cmake" "lib${PROJECT_NAME}.pc" @ONLY)

# Install library and header
install(TARGETS unarr
        EXPORT unarr-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install pkg-config file
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib${PROJECT_NAME}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Export and install targets
install(EXPORT unarr-targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unarr
        NAMESPACE unarr::)

# Make project importable from build dir
export(TARGETS unarr
        FILE unarr-targets.cmake
        NAMESPACE unarr::)

# Write a config file for installation
configure_package_config_file(
        unarr-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/unarr-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unarr)

# Write version file
write_basic_package_version_file(unarr-version.cmake
        COMPATIBILITY AnyNewerVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/unarr-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/unarr-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unarr)
