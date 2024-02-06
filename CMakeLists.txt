cmake_minimum_required(VERSION 3.12.0)

project(AzurLane)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX)
add_compile_options(-fno-rtti)
add_link_options(-s)

add_definitions(-DANDROID_ABI="${ANDROID_ABI}")

find_library(liblog log NO_CACHE)

include_directories(
    .
    third_party
)

# Build dobby library
set(DOBBY_GENERATE_SHARED OFF)
set(Plugin.SymbolResolver OFF)
add_subdirectory(third_party/Dobby)

# Build cheat library
add_library(${PROJECT_NAME} SHARED src/main.cc)
target_link_libraries(${PROJECT_NAME} ${liblog} dobby_static)
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/binaries/${ANDROID_ABI}"
)
