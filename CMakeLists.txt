cmake_minimum_required(VERSION 3.0)
project(unarr VERSION 1.0 LANGUAGES C)

set(PROJECT_DESCRIPTION
  "A lightweight decompression library for rar, tar and zip files.")

# Set up api and release version for later use.
# Increase in major api version indicates api
# breakage!! For non-breaking changes, use
# minor api version instead.
set(UNARR_API_VERSION_MAJOR 1)
set(UNARR_API_VERSION_MINOR 0)

include(GNUInstallDirs)

# Set build type to default if unset.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

option(BUILD_SHARED_LIBS "Build ${PROJECT_NAME} as a shared library" ON)

if(BUILD_SHARED_LIBS)
  set(CMAKE_C_VISIBILITY_PRESET hidden)
  add_definitions(-DUNARR_IS_SHARED_LIBRARY -DUNARR_EXPORT_SYMBOLS)
endif()

find_package(ZLIB)
find_package(BZip2)
find_package(LibLZMA)

if(UNIX OR MINGW OR MSYS)
  add_compile_options(-Wall -Wextra -pedantic
                      -Wstrict-prototypes -Wmissing-prototypes
                      -Werror-implicit-function-declaration
                      -fomit-frame-pointer -flto)
  add_definitions(-D_FILE_OFFSET_BITS=64)

  if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-Wno-missing-field-initializers)
    list(APPEND CMAKE_SHARED_LINKER_FLAGS -flto)
    list(APPEND CMAKE_STATIC_LINKER_FLAGS -flto)
  endif()
endif()

# Sources

set(HEADERS common/allocator.h
  common/unarr-imp.h
  rar/rar.h
  rar/lzss.h
  rar/rarvm.h
  lzmasdk/Ppmd7.h
  lzmasdk/CpuArch.h
  lzmasdk/Ppmd.h
  lzmasdk/7zTypes.h
  lzmasdk/Ppmd8.h
  lzmasdk/Precomp.h
  _7z/_7z.h
  zip/zip.h
  zip/inflate.h
  tar/tar.h)

set(SOURCES rar/uncompress-rar.c
  rar/huffman-rar.c
  rar/rar.c
  rar/filter-rar.c
  rar/rarvm.c
  rar/parse-rar.c
  _7z/_7z.c
  zip/zip.c
  zip/inflate.c
  zip/parse-zip.c
  zip/uncompress-zip.c
  tar/tar.c
  tar/parse-tar.c
  lzmasdk/Ppmd7.c
  lzmasdk/Ppmd8.c
  lzmasdk/CpuArch.c
  #lzmasdk/LzmaDec.c
  lzmasdk/Ppmd7Dec.c
  lzmasdk/Ppmd8Dec.c
  #common/custalloc.c
  common/unarr.c
  common/stream.c
  common/conv.c
  common/crc32.c
  )

if(NOT LIBLZMA_FOUND)
  list(APPEND HEADERS lzmasdk/LzmaDec.h)
  list(APPEND SOURCES lzmasdk/LzmaDec.c)
endif()

# Build targets

add_library(unarr unarr.h ${HEADERS} ${SOURCES})
set_target_properties(unarr PROPERTIES VERSION
                      ${UNARR_API_VERSION_MAJOR}.${UNARR_API_VERSION_MINOR}
                      SOVERSION ${UNARR_API_VERSION_MAJOR})

# Check if zlib and bzip2 are present.
# If yes, use them.

if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
  target_link_libraries(unarr ${ZLIB_LIBRARIES})
  add_definitions(-DHAVE_ZLIB)
  # Add zlib to libunarr.pc Requires.private
  set (PKG_CONFIG_REQUIRES_PRIVATE "${PKG_CONFIG_REQUIRES_PRIVATE} zlib")
endif()

if(BZIP2_FOUND)
  include_directories(${BZIP_INCLUDE_DIRS})
  target_link_libraries (unarr ${BZIP2_LIBRARIES})
  add_definitions(-DHAVE_BZIP2)
  # Bzip2 upstream does not supply a .pc file. Add it to Libs.private.
  set (PKG_CONFIG_LIBS_PRIVATE "-I${BZIP_INCLUDE_DIRS} -l${BZIP2_LIBRARIES}")
endif()

if(LIBLZMA_FOUND)
  include_directories(${LIBLZMA_INCLUDE_DIRS})
  target_link_libraries(unarr ${LIBLZMA_LIBRARIES})
  add_definitions(-DHAVE_LIBLZMA)
  set (PKG_CONFIG_REQUIRES_PRIVATE "${PKG_CONFIG_REQUIRES_PRIVATE} liblzma")
endif()

# Variables for libunarr.pc pkg-config file

set(PKG_CONFIG_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
set(PKG_CONFIG_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
set(PKG_CONFIG_LIBS "-L\${libdir} -l${PROJECT_NAME}")
set(PKG_CONFIG_CFLAGS "-I\${includedir}")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/pkg-config.pc.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.pc"
)

# Targets for make install
install(TARGETS unarr DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES unarr.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libunarr.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
