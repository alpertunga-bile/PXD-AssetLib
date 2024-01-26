cmake_minimum_required(VERSION 3.5.0)
project(pxd-assetlib VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

################################################################################
## GLOBAL DEFINITIONS

set(CMAKE_BUILD_TYPE Debug)

set(TEST_PROJECT_NAME asset-test)

add_definitions(
    -DGLM_ENABLE_SIMD_AVX2=ON 
    -DGLM_ENABLE_CXX_20=ON
    -DASSIMP_DOUBLE_PRECISION=ON
    )

if((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	add_compile_options(-mavx2)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
	add_compile_options(/QxAVX2)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	add_compile_options(/arch:AVX2)
endif()

set(HEADER_FILES
    includes/debug.hpp
    includes/scene.hpp
    includes/assimp_importer.hpp
    includes/fastgltf_importer.hpp
    includes/types.hpp
    third-party/assimp/include/assimp/Importer.hpp
    third-party/assimp/include/assimp/postprocess.h
    third-party/assimp/include/assimp/scene.h
    third-party/glm/glm/gtx/quaternion.hpp
    third-party/glm/glm/mat4x4.hpp
    third-party/glm/glm/vec4.hpp
    third-party/fastgltf/include/fastgltf/parser.hpp
    third-party/fastgltf/include/fastgltf/tools.hpp
    third-party/fastgltf/include/fastgltf/glm_element_traits.hpp
    third-party/stb-image/stb_image.h
    third-party/meshoptimizer/src/meshoptimizer.h
)
    
set(SOURCE_FILES
    sources/scene.cpp
    sources/debug.cpp
    sources/assimp_importer.cpp
    sources/fastgltf_importer.cpp
    ${HEADER_FILES}
)

include_directories(
    includes
    third-party/stb-image
    third-party/fastgltf/include/fastgltf
    third-party/meshoptimizer/src
    third-party/glm/glm
    third-party/assimp/include
)


################################################################################
## STATIC LIBRARY DEFINITONS

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} fastgltf meshoptimizer glm assimp)

target_precompile_headers(
    ${PROJECT_NAME} PRIVATE
    ${HEADER_FILES}
    <string_view>
    <filesystem>
    <vector>
    <cstdio>
    <format>
    <string>
    <unordered_map>
    <algorithm>
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

################################################################################
## TEST EXECUTABLE DEFINITIONS

add_executable(${TEST_PROJECT_NAME} main.cpp ${SOURCE_FILES})
target_link_libraries(${TEST_PROJECT_NAME} fastgltf meshoptimizer glm assimp)

target_precompile_headers(
    ${TEST_PROJECT_NAME} PRIVATE
    ${HEADER_FILES}
    <string_view>
    <filesystem>
    <vector>
    <cstdio>
    <format>
    <string>
    <unordered_map>
    <algorithm>
)

target_compile_features(${TEST_PROJECT_NAME} PRIVATE cxx_std_20)

add_subdirectory(third-party/fastgltf)
add_subdirectory(third-party/meshoptimizer)
add_subdirectory(third-party/glm)
add_subdirectory(third-party/assimp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
