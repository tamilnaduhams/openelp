cmake_minimum_required(VERSION 2.7)

include(ExternalProject)

set(PCRE_TARGET_VERSION "10.21")

set(PCRE_CMAKE_ARGS
  -DBUILD_SHARED_LIBS:BOOL=OFF
  -DBUILD_STATIC_LIBS:BOOL=ON
  -DPCRE2_BUILD_PCRE2_8:BOOL=ON
  -DPCRE2_BUILD_PCRE2_16:BOOL=OFF
  -DPCRE2_BUILD_PCRE2_32:BOOL=OFF
  -DPCRE2_SUPPORT_JIT:BOOL=OFF
  -DPCRE2_BUILD_PCRE2GREP:BOOL=OFF
  -DPCRE2_BUILD_TESTS:BOOL=OFF
  )

if(DEFINED CMAKE_VERBOSE_MAKEFILE)
  set(PCRE_CMAKE_ARGS ${PCRE_CMAKE_ARGS}
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    )
endif()

if(WIN32)
  string(REGEX REPLACE
    "( |^)/W[0-4]( |$)" "\\1"
    CMAKE_C_FLAGS_FIXED
    ${CMAKE_C_FLAGS}
    )
  set(PCRE_CMAKE_ARGS ${PCRE_CMAKE_ARGS}
    "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS_FIXED}"
    )
else()
  set(PCRE_CMAKE_ARGS ${PCRE_CMAKE_ARGS}
    "-DCMAKE_C_FLAGS:STRING=-fPIC ${CMAKE_C_FLAGS}"
    )
endif()

if(${CMAKE_VERSION} VERSION_LESS 3.0)
  set(PCRE_PATCH_CMD git apply --ignore-whitespace -p1 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/pcre2_cmp0026.patch")
else()
  set(PCRE_PATCH_CMD "")
endif()

ExternalProject_Add(pcre
  URL "ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre2-${PCRE_TARGET_VERSION}.tar.gz"
  URL_MD5 b75fcdcce309c9778d1a5733b591c5db
  CMAKE_ARGS ${PCRE_CMAKE_ARGS}
  PATCH_COMMAND ${PCRE_PATCH_CMD}
  INSTALL_COMMAND ""
  )

ExternalProject_Get_Property(pcre
  SOURCE_DIR
  BINARY_DIR
  )

set(OPENELP_PCRE_LICENSE_PATH "${SOURCE_DIR}/LICENCE")

set(PCRE_INCLUDE_DIRS "${BINARY_DIR}")
set(PCRE_LIBRARY_DIRS "")
if(WIN32)
  set(PCRE_LIBRARIES "${BINARY_DIR}/$(Configuration)/pcre2-8*.lib")
else()
  set(PCRE_LIBRARIES "${BINARY_DIR}/libpcre2-8.a")
endif()
