cmake_minimum_required(VERSION 3.5.0)
project(pxd-assetlib VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

################################################################################
## GLOBAL DEFINITIONS

set(BUILD_SHARED_LIBS OFF)

include(setup.cmake)

set_debug()
set_cxx(20)
enable_avx2()
enable_iwyu()

set(PXD_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party)
set(PXD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(PXD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sources)

# set PXD-STL
set(PXD_STL_BUILD_TEST_EXECUTABLE OFF)
add_subdirectory(${PXD_THIRD_PARTY_DIR}/PXD-STL)

set(PXD_STL_INCLUDE_DIR ${PXD_THIRD_PARTY_DIR}/PXD-STL/includes)

set(TEST_PROJECT_NAME asset-test)

# GLM Options
set(GLM_ENABLE_SIMD_AVX2 ON)
set(GLM_ENABLE_CXX_20 ON)

# ASSIMP Project Options
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_NO_EXPORT ON)
set(BUILD_SHARED_LIBS OFF)

set(PXD_HEADER_FILES
    ${PXD_INCLUDE_DIR}/model.hpp
    ${PXD_INCLUDE_DIR}/assimp_importer.hpp
    ${PXD_INCLUDE_DIR}/fastgltf_importer.hpp
    ${PXD_INCLUDE_DIR}/types.hpp

    ${PXD_STL_INCLUDE_DIR}/logger.hpp

    ${PXD_THIRD_PARTY_DIR}/assimp/include/assimp/Importer.hpp
    ${PXD_THIRD_PARTY_DIR}/assimp/include/assimp/postprocess.h
    ${PXD_THIRD_PARTY_DIR}/assimp/include/assimp/scene.h
    ${PXD_THIRD_PARTY_DIR}/glm/glm/gtx/quaternion.hpp
    ${PXD_THIRD_PARTY_DIR}/glm/glm/mat4x4.hpp
    ${PXD_THIRD_PARTY_DIR}/glm/glm/vec4.hpp
    ${PXD_THIRD_PARTY_DIR}/fastgltf/include/fastgltf/core.hpp
    ${PXD_THIRD_PARTY_DIR}/fastgltf/include/fastgltf/tools.hpp
    ${PXD_THIRD_PARTY_DIR}/fastgltf/include/fastgltf/glm_element_traits.hpp
    ${PXD_THIRD_PARTY_DIR}/stb-image/stb_image.h
    ${PXD_THIRD_PARTY_DIR}/meshoptimizer/src/meshoptimizer.h
)
    
set(PXD_SOURCE_FILES
    ${PXD_SOURCE_DIR}/model.cpp
    ${PXD_SOURCE_DIR}/assimp_importer.cpp
    ${PXD_SOURCE_DIR}/fastgltf_importer.cpp
    ${PXD_SOURCE_DIR}/types.cpp
    ${PXD_HEADER_FILES}
)

include_directories(
    ${PXD_INCLUDE_DIR}
    ${PXD_STL_INCLUDE_DIR}
    ${PXD_THIRD_PARTY_DIR}/stb-image
    ${PXD_THIRD_PARTY_DIR}/fastgltf/include/fastgltf
    ${PXD_THIRD_PARTY_DIR}/meshoptimizer/src
    ${PXD_THIRD_PARTY_DIR}/glm/glm
    ${PXD_THIRD_PARTY_DIR}/assimp/include
)

add_subdirectory(${PXD_THIRD_PARTY_DIR}/fastgltf)
add_subdirectory(${PXD_THIRD_PARTY_DIR}/meshoptimizer)
add_subdirectory(${PXD_THIRD_PARTY_DIR}/glm)
add_subdirectory(${PXD_THIRD_PARTY_DIR}/assimp)

set(LIBS_TO_LINK
    fastgltf
    meshoptimizer
    glm
    assimp::assimp
    pxd-stl
)

################################################################################
## STATIC LIBRARY DEFINITONS

add_library(${PROJECT_NAME} ${PXD_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${LIBS_TO_LINK})

target_precompile_headers(
    ${PROJECT_NAME} PRIVATE
    ${PXD_HEADER_FILES}
)

################################################################################
## TEST EXECUTABLE DEFINITIONS

add_executable(${TEST_PROJECT_NAME} main.cpp ${PXD_SOURCE_FILES})
target_link_libraries(${TEST_PROJECT_NAME} ${LIBS_TO_LINK})

target_precompile_headers(
    ${TEST_PROJECT_NAME} PRIVATE
    ${PXD_HEADER_FILES}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
