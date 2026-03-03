project(argon2 LANGUAGES C)

set(ARGON2_PUBLIC_HEADERS
    include/argon2.h
)

set(ARGON2_SOURCES
    src/argon2.c
    src/core.c
    src/blake2/blake2b.c
    src/thread.c
    src/encoding.c
    src/opt.c
)

add_library(argon2 STATIC ${ARGON2_SOURCES})
set_target_properties(argon2 PROPERTIES PUBLIC_HEADER ${ARGON2_PUBLIC_HEADERS})
target_include_directories(argon2
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(argon2::argon2 ALIAS argon2)
