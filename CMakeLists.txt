cmake_minimum_required (VERSION 3.2)

project("rpc-lite" VERSION 1.0.0)
find_package(Threads REQUIRED)
include(CMakePackageConfigHelpers)

if(WIN32)
   set(Sock_libs ws2_32 IPHLPAPI)
   add_definitions(-DWIN32)
endif()

add_compile_options($<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall>
                    $<$<CXX_COMPILER_ID:MSVC>:/W3>)
                    
add_subdirectory(lib)
add_subdirectory(examples)

option(USE_SANITIZER_ADDRESS "Address access and memory leaks sanitizer" OFF)
option(USE_SANITIZER_THREAD "Data race sanitizer" OFF)
option(USE_SANITIZER_UNDEFINED "Undefined behaviour detector" OFF)

if (USE_SANITIZER_ADDRESS)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()
if (USE_SANITIZER_THREAD)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()
if (USE_SANITIZER_UNDEFINED)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()
