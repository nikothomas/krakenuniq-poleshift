cmake_minimum_required(VERSION 3.15)
project(krakenuniq CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(OpenMP)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wfatal-errors -pipe -O2")
if(NDEBUG)
    add_definitions(-DNDEBUG)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/gzstream
    ${CMAKE_SOURCE_DIR}/src/klib
    ${CMAKE_SOURCE_DIR}/src/third_party
)

# Add library sources
set(LIB_SOURCES
    src/krakendb.cpp
    src/quickfile.cpp
    src/krakenutil.cpp
    src/seqreader.cpp
    src/uid_mapping.cpp
    src/hyperloglogplus.cpp
    src/gzstream/gzstream.C
    src/third_party/MurmurHash3.cpp
)

# Create library
add_library(krakenuniq_lib STATIC ${LIB_SOURCES})
target_link_libraries(krakenuniq_lib PRIVATE
    ZLIB::ZLIB
    BZip2::BZip2
    $<$<BOOL:${OpenMP_CXX_FOUND}>:OpenMP::OpenMP_CXX>
)

# Define executables
set(EXECUTABLES
    classify
    db_sort
    set_lcas
    db_shrink
    build_taxdb
    read_uid_mapping
    count_unique
    dump_taxdb
    query_taxdb
)

# Build executables
foreach(EXEC ${EXECUTABLES})
    add_executable(${EXEC} src/${EXEC}.cpp)
    target_link_libraries(${EXEC} PRIVATE krakenuniq_lib)
endforeach()

# Build classifyExact separately with special flag
add_executable(classifyExact src/classify.cpp)
target_compile_definitions(classifyExact PRIVATE EXACT_COUNTING)
target_link_libraries(classifyExact PRIVATE krakenuniq_lib)

# Install targets
install(TARGETS ${EXECUTABLES} classifyExact
    RUNTIME DESTINATION bin
)

# Optional testing
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    # Add individual test targets here when they're ready
    if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test.sh")
        add_test(NAME integration_test
            COMMAND ${CMAKE_SOURCE_DIR}/tests/test.sh
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests
        )
    endif()
endif()