cmake_minimum_required(VERSION 2.6)
project(unarr)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

#set build type to default if unset
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release CACHE STRING
       "Choose the type of build, options are: None Debug Release RelWithDebInfo
MinSizeRel."
       FORCE )
endif()

find_package(ZLIB)
find_package(BZip2)
find_package(Threads)

if (UNIX OR MINGW)
    add_compile_options(-std=c99 -fomit-frame-pointer)
endif (UNIX OR MINGW)

include_directories(common rar tar zip _7z)

set (HEADERS 	common/allocator.h
			common/unarr-imp.h
			rar/rar.h
			rar/lzss.h
			rar/rarvm.h
			lzmasdk/LzmaDec.h
			lzmasdk/Ppmd7.h
			lzmasdk/CpuArch.h
			lzmasdk/Ppmd.h
			lzmasdk/Types.h
			lzmasdk/Ppmd8.h
			_7z/_7z.h
			zip/zip.h
			zip/inflate.h
			tar/tar.h)

set (SOURCES	rar/uncompress-rar.c
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
			lzmasdk/LzmaDec.c
			lzmasdk/Ppmd7Dec.c
			lzmasdk/Ppmd8Dec.c
			common/custalloc.c
			common/unarr.c
			common/stream.c
			common/conv.c
			common/crc32.c
			)

add_library(unarr SHARED unarr.h ${HEADERS} ${SOURCES})
if (UNIX)
    target_link_libraries(unarr m)
endif (UNIX)

if (Threads_FOUND)
    target_link_libraries(unarr ${CMAKE_THREAD_LIBS_INIT})
endif (Threads_FOUND)

if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(unarr ${ZLIB_LIBRARIES})
    add_definitions(-DHAVE_ZLIB)
endif(ZLIB_FOUND)

if (BZIP2_FOUND)
    include_directories(${BZIP_INCLUDE_DIRS})
    target_link_libraries (unarr ${BZIP2_LIBRARIES})
    add_definitions(-DHAVE_BZIP2)
endif (BZIP2_FOUND)

install(TARGETS unarr DESTINATION lib)
install(FILES unarr.h DESTINATION include)