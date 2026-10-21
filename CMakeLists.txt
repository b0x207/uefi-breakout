cmake_minimum_required(VERSION 3.20)
project("breakout"
        VERSION 0.0
        LANGUAGES C
)

# Default to full debug mode
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE CACHE STRING Debug FORCE)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
set(FETCHCONTENT_QUIET OFF)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-fcolor-diagnostics)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(SANITIZER_FLAGS "-fsanitize=address,undefined,leak")
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(${SANITIZER_FLAGS} -fno-omit-frame-pointer)
        add_link_options(${SANITIZER_FLAGS})
    elseif(MSVC)
        add_compile_options(/fsanitize=address)
    endif()
endif()

add_executable(breakout.efi)
target_include_directories(breakout.efi PRIVATE include)
target_sources(
    breakout.efi PRIVATE
    main.c
)
