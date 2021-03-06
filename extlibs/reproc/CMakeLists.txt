# Every variable is namespaced with REPROC and everything except for the library
# itself is turned off by default to make reproc easy to use as a CMake
# subproject with add_subdirectory

cmake_minimum_required(VERSION 3.1)
project(reproc VERSION 1.0.0 LANGUAGES C)

# Tip: use cmake-gui when contributing to reproc to turn on everything without
# having to type out all the options by hand (cmake .. && cmake-gui .)
option(REPROC_BUILD_TESTS "Build tests" OFF)
option(REPROC_BUILD_EXAMPLES "Build examples. Enable REPROC_BUILD_CXX_WRAPPER \
to build C++ examples." OFF)
option(REPROC_BUILD_CXX_WRAPPER "Build C++ wrapper" OFF)
option(REPROC_CI "Add -Werror or equivalent to compiler and clang-tidy" OFF)
option(REPROC_TIDY "Run clang-tidy as part of the build \
(requires clang-tidy to be in the PATH)" OFF)
option(REPROC_INSTALL "Generate install target" OFF)
option(REPROC_FORMAT "Find clang-format and add target to run it on all source \
files" OFF)
option(REPROC_SANITIZERS "Compile with sanitizers" OFF)
# CMake adds /W3 to CMAKE_C_FLAGS and CMAKE_CXX_FLAGS by default on Windows so
# when we add /W4 via target_compile_options cl.exe complains that both /W3 and
# /W4 are passed. We can avoid these warnings by replacing /W3 with /W4 in
# CMAKE_CXX_FLAGS but this is intrusive when reproc is used via add_subdirectory
# since it would add /W4 for every target. To solve this, we add this option so
# /W4 can be enabled by developers and in CI without forcing it on for users.
option(REPROC_ENABLE_W4 "Enable /W4 on Windows (modifies CMAKE_C_FLAGS and \
CMAKE_CXX_FLAGS)" OFF)

# C++

if(REPROC_BUILD_CXX_WRAPPER OR REPROC_BUILD_TESTS)
  enable_language(CXX)
endif()

### clang-tidy ###

if(REPROC_TIDY)
  cmake_minimum_required(VERSION 3.6)
  find_program(REPROC_CLANG_TIDY clang-tidy)

  if(REPROC_CLANG_TIDY)
    if (REPROC_CI)
      set(REPROC_CLANG_TIDY ${REPROC_CLANG_TIDY} -warnings-as-errors=*)
    endif()
  else()
    message(WARNING "clang-tidy not found")
  endif()
endif()

### Common development flags ###

if(MSVC)
  if(REPROC_ENABLE_W4)
    string(REGEX REPLACE /W[0-4] "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4" )

    string(REGEX REPLACE /W[0-4] "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4" )
  endif()

  include(CheckCCompilerFlag)
  check_c_compiler_flag(/permissive- REPROC_HAS_PERMISSIVE)

  set(REPROC_COMPILE_OPTIONS
    /nologo
    $<$<BOOL:${REPROC_CI}>:/WX>
    $<$<BOOL:${REPROC_HAS_PERMISSIVE}>:/permissive->
    # MSVC reports non-constant initializers as a nonstandard extension but
    # they've been standardized in C99 and we're targeting C99 so this warning
    # is not valid for us
    $<$<C_COMPILER_ID:MSVC>:/wd4204>
  )
  set(REPROC_LINK_OPTIONS
    /nologo
    # Disable incremental linking to silence warnings when rebuilding reproc
    /INCREMENTAL:NO
  )
else()
  set(REPROC_COMPILE_OPTIONS
    -Wall
    -Wextra
    -pedantic-errors
    -Wshadow
    -Wconversion
    $<$<BOOL:${REPROC_CI}>:-Werror>
  )

  if(REPROC_SANITIZERS)
    set(REPROC_COMPILE_OPTIONS ${REPROC_COMPILE_OPTIONS}
      -fsanitize=address,undefined
    )

    set(REPROC_LINK_OPTIONS -fsanitize=address,undefined)

    if(${CMAKE_C_COMPILER_ID} STREQUAL GNU)
      set(REPROC_LINK_OPTIONS ${REPROC_LINK_OPTIONS} -fuse-ld=gold)
    endif()
  endif()
endif()

string(REPLACE ";" " " REPROC_LINK_OPTIONS_STRING "${REPROC_LINK_OPTIONS}")

### Helpers ###

set(REPROC_C_STANDARD 99)
set(REPROC_CXX_STANDARD 11)

function(reproc_set_common TARGET LANGUAGE OUTPUT_DIRECTORY)
  string(REPLACE "reproc-" "" WITHOUT_PREFIX ${TARGET})

  set_target_properties(${TARGET} PROPERTIES
    ${LANGUAGE}_STANDARD ${REPROC_${LANGUAGE}_STANDARD}
    ${LANGUAGE}_STANDARD_REQUIRED ON
    ${LANGUAGE}_EXTENSIONS OFF
    OUTPUT_NAME ${WITHOUT_PREFIX}
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
  )
  target_compile_options(${TARGET} PRIVATE ${REPROC_COMPILE_OPTIONS})

  # reproc_set_common is called twice for reproc target (once for C and once for
  # C++) which means we have to take care to not add link options twice
  get_property(LINK_FLAGS TARGET ${TARGET} PROPERTY LINK_FLAGS)
  if(NOT "${LINK_FLAGS}" MATCHES "${REPROC_LINK_OPTIONS_STRING}")
    set_property(TARGET ${TARGET} PROPERTY
                 LINK_FLAGS "${LINK_FLAGS} ${REPROC_LINK_OPTIONS_STRING}")
  endif()

  if(REPROC_TIDY AND REPROC_CLANG_TIDY)
    set_target_properties(${TARGET} PROPERTIES
      # REPROC_CLANG_TIDY is a list so we surround it with quotes to pass it as
      # a single argument
      ${LANGUAGE}_CLANG_TIDY "${REPROC_CLANG_TIDY}"
    )
  endif()
endfunction()

### Library ###

add_library(reproc "")
add_library(reproc::reproc ALIAS reproc)

reproc_set_common(reproc C lib)

if(NOT MSVC)
  target_compile_options(reproc PRIVATE
    # More or less dllexport/dllimport for GCC and Clang
    -fvisibility=hidden
  )
endif()

set_target_properties(reproc PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Silence MSVC Library Manager version output
if(MSVC)
  set_property(TARGET reproc APPEND PROPERTY STATIC_LIBRARY_FLAGS /nologo)
endif()

# Only use the headers from the repository when building. When installing we
# want to use the install location of the headers (e.g. /usr/include) as the
# include directory which is specified later in this file (Install section)
target_include_directories(reproc PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/c>
)

# Used to add visibility modifiers (declspec)
target_compile_definitions(reproc
  PUBLIC $<$<BOOL:${BUILD_SHARED_LIBS}>:REPROC_SHARED>
  PRIVATE $<BUILD_INTERFACE:REPROC_BUILDING>
)

target_sources(reproc PRIVATE src/c/common.c)

if(WIN32)
  target_include_directories(reproc PRIVATE src/c/windows)
  target_sources(reproc PRIVATE
    src/c/windows/handle.c
    src/c/windows/pipe.c
    src/c/windows/process.c
    src/c/windows/reproc.c
    src/c/windows/string_utils.c
  )
elseif(UNIX)
  target_include_directories(reproc PRIVATE src/c/posix)
  target_sources(reproc PRIVATE
    src/c/posix/fork.c
    src/c/posix/pipe.c
    src/c/posix/reproc.c
    src/c/posix/process.c
  )
endif()

if(MSVC)
  # Check if STARTUPINFOEX and related functions are available
  include(CheckSymbolExists)
  check_symbol_exists(InitializeProcThreadAttributeList windows.h
                      REPROC_ATTRIBUTE_LIST_FOUND)

  target_compile_definitions(reproc PRIVATE
    _CRT_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
    VC_EXTRALEAN
    NOMINMAX
    $<$<BOOL:${REPROC_ATTRIBUTE_LIST_FOUND}>:HAS_ATTRIBUTE_LIST>
  )
elseif(UNIX)
  # Check if pipe2 is available
  include(CheckSymbolExists)
  list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
  check_symbol_exists(pipe2 unistd.h REPROC_PIPE2_FOUND)
  list(REMOVE_AT CMAKE_REQUIRED_DEFINITIONS -1)

  target_compile_definitions(reproc PRIVATE
    _GNU_SOURCE
    $<$<BOOL:${REPROC_PIPE2_FOUND}>:HAS_PIPE2>
  )
endif()

### C++ Wrapper ###

if(REPROC_BUILD_CXX_WRAPPER)
  reproc_set_common(reproc CXX lib)

  target_include_directories(reproc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/cpp>
  )
  target_sources(reproc PRIVATE src/cpp/reproc.cpp src/cpp/error.cpp)
endif()

### Tests ###

if(REPROC_BUILD_TESTS)
  add_executable(reproc-tests "")
  reproc_set_common(reproc-tests CXX test)

  target_link_libraries(reproc-tests PRIVATE reproc)
  target_include_directories(reproc-tests SYSTEM PRIVATE third-party/doctest)

  target_sources(reproc-tests PRIVATE
    test/impl.cpp
    test/read-write.cpp
    test/stop.cpp
    test/working-directory.cpp
  )

  ### Helper programs ###

  function(reproc_add_test_helper TARGET)
    add_executable(reproc-${TARGET} test/res/${TARGET}.cpp)
    reproc_set_common(reproc-${TARGET} CXX test/res)

    string(TOUPPER ${TARGET} TARGET_UPPER_CASE)
    # Pass paths as compile time parameters so tests can be run from any
    # directory without hardcoding the path in the tests which differs for each
    # user.
    target_compile_definitions(reproc-tests PRIVATE
      ${TARGET_UPPER_CASE}_PATH="$<TARGET_FILE:reproc-${TARGET}>"
      ${TARGET_UPPER_CASE}_DIR="$<TARGET_FILE_DIR:reproc-${TARGET}>"
    )
    add_dependencies(reproc-tests reproc-${TARGET})
  endfunction()

  reproc_add_test_helper(stdout)
  reproc_add_test_helper(stderr)
  reproc_add_test_helper(infinite)
  reproc_add_test_helper(noop)

  add_custom_target(
    reproc-run-tests
    COMMAND $<TARGET_FILE:reproc-tests> --force-colors=true
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_dependencies(reproc-run-tests reproc-tests)
endif()

### Examples ###

if(REPROC_BUILD_EXAMPLES)

  set(REPROC_C_EXT c)
  set(REPROC_CXX_EXT cpp)

  function(reproc_add_example TARGET LANGUAGE)
    add_executable(reproc-${TARGET} example/${TARGET}.${REPROC_${LANGUAGE}_EXT})
    reproc_set_common(reproc-${TARGET} ${LANGUAGE} example)

    target_link_libraries(reproc-${TARGET} PRIVATE reproc)
    if(REPROC_SANITIZERS)
      set_target_properties(reproc-${TARGET} PROPERTIES
        # Hack to avoid UBSAN undefined reference errors
        LINKER_LANGUAGE CXX
      )
    endif()
  endfunction()

  reproc_add_example(git-status C)

  if(REPROC_BUILD_CXX_WRAPPER)
    reproc_add_example(cmake-help CXX)
    reproc_add_example(forward CXX)
  endif()
endif()

### Install ###

# Adapted from https://codingnest.com/basic-cmake-part-2/

if(REPROC_INSTALL)

  include(GNUInstallDirs)
  set(REPROC_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/reproc)

  install(
    TARGETS reproc
    EXPORT reproc-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  target_include_directories(reproc PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  install(
    EXPORT reproc-targets
    FILE reproc-targets.cmake
    NAMESPACE reproc::
    DESTINATION ${REPROC_INSTALL_CONFIGDIR}
  )

  install(
    DIRECTORY include/c/reproc
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  if(REPROC_BUILD_CXX_WRAPPER)
    install(
      DIRECTORY include/cpp/reproc
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()

  include(CMakePackageConfigHelpers)
  write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/reproc-config-version.cmake
      VERSION ${PROJECT_VERSION}
      COMPATIBILITY AnyNewerVersion
  )

  configure_package_config_file(
      ${CMAKE_CURRENT_LIST_DIR}/cmake/reproc-config.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/reproc-config.cmake
      INSTALL_DESTINATION ${REPROC_INSTALL_CONFIGDIR}}
  )

  install(
      FILES
        ${CMAKE_CURRENT_BINARY_DIR}/reproc-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/reproc-config-version.cmake
      DESTINATION ${REPROC_INSTALL_CONFIGDIR}
  )

endif()

### clang-format ###

if(REPROC_FORMAT)
  find_program(REPROC_CLANG_FORMAT clang-format)
  if(REPROC_CLANG_FORMAT)
    file(GLOB_RECURSE REPROC_SOURCES
      include/*.[h,c,hpp,cpp]
      src/*.[h,c,hpp,cpp]
      test/*.[h,c,hpp,cpp]
      example/*.[h,c,hpp,cpp]
    )

    add_custom_target(
      reproc-format
      COMMAND ${REPROC_CLANG_FORMAT} -i ${REPROC_SOURCES}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  else()
    message(WARNING "clang-format not found")
  endif()
endif()
