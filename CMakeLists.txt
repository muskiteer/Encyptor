cmake_minimum_required(VERSION 3.16)
project(encryptor VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags for better security and debugging
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wall -Wextra")

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)

# Add executable
add_executable(encryptor 
    main.cpp
)

# Link libraries
target_link_libraries(encryptor 
    OpenSSL::SSL 
    OpenSSL::Crypto
    ${LIBZIP_LIBRARIES}
)

# Include directories
target_include_directories(encryptor PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBZIP_INCLUDE_DIRS}
)

# Compiler flags
target_compile_options(encryptor PRIVATE ${LIBZIP_CFLAGS_OTHER})

# Set output directory
set_target_properties(encryptor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME "encryptor"
)

# Add install target
install(TARGETS encryptor DESTINATION bin)