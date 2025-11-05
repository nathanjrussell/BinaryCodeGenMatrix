cmake_minimum_required(VERSION 3.16)
# Read project version from `VERSION.txt` and use it in project()
set(VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt")
if(NOT EXISTS "${VERSION_FILE}")
  message(FATAL_ERROR "Missing ${VERSION_FILE} - create it with the project version (e.g. 2.0.2)")
endif()

file(READ "${VERSION_FILE}" _version_raw)
string(STRIP "${_version_raw}" PROJECT_VERSION)

if(NOT PROJECT_VERSION MATCHES "[0-9]\\.[0-9]\\.[0-9]")
  message(WARNING "Read '${PROJECT_VERSION}' from ${VERSION_FILE} which may not match X[.Y][.Z] format")
endif()

project(BinaryCodeGenMatrix VERSION ${PROJECT_VERSION} LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/external_repos.cmake)
#find_package(BinaryCodeWord v2.5.0 QUIET)

#if(NOT BinaryCodeWord_FOUND)
#  include(FetchContent)
#  set(BINARY_CODE_WORD_GIT_URL "https://github.com/nathanjrussell/BinaryCodeWord.git")
#  set(BINARY_CODE_WORD_VERSION "2.0.2")
#  message(STATUS "BinaryCodeWord not found; fetching from ${BINARY_CODE_WORD_GIT_URL} tag v${BINARY_CODE_WORD_VERSION}")
#  FetchContent_Declare(
#    BinaryCodeWordProj
#    GIT_REPOSITORY ${BINARY_CODE_WORD_GIT_URL}
#    GIT_TAG        v${BINARY_CODE_WORD_VERSION}
#  )
#  FetchContent_MakeAvailable(BinaryCodeWordProj)
#endif()

add_library(binarycodegenmat src/BinaryCodeGenMat.cpp)
add_library(BinaryCodeGenMat::binarycodegenmat ALIAS binarycodegenmat)
target_compile_features(binarycodegenmat PUBLIC cxx_std_17)
target_include_directories(binarycodegenmat
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(binarycodegenmat PUBLIC BinaryCodeWord::binarycodeword)

add_executable(genmat_basic_example examples/basic_usage.cpp)
target_link_libraries(genmat_basic_example PRIVATE BinaryCodeGenMat::binarycodegenmat)
target_compile_features(genmat_basic_example PRIVATE cxx_std_17)

include(CTest)
option(BINARYCODEGENMAT_BUILD_TESTS "Build unit tests" ON)
if(BINARYCODEGENMAT_BUILD_TESTS AND BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  add_executable(test_BinaryCodeGenMat tests/test_BinaryCodeGenMat.cpp)
  target_link_libraries(test_BinaryCodeGenMat PRIVATE BinaryCodeGenMat::binarycodegenmat GTest::gtest_main)
  target_compile_features(test_BinaryCodeGenMat PRIVATE cxx_std_17)
  include(GoogleTest)
  gtest_discover_tests(test_BinaryCodeGenMat)
endif()

include(CMakePackageConfigHelpers)
set(INSTALL_CONFIGDIR lib/cmake/BinaryCodeGenMat)
install(TARGETS binarycodegenmat
  EXPORT BinaryCodeGenMatTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT BinaryCodeGenMatTargets
  FILE BinaryCodeGenMatTargets.cmake
  NAMESPACE BinaryCodeGenMat::
  DESTINATION ${INSTALL_CONFIGDIR})
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/BinaryCodeGenMatConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BinaryCodeGenMatConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/BinaryCodeGenMatConfig.cmake"
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR})
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/BinaryCodeGenMatConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/BinaryCodeGenMatConfigVersion.cmake"
  DESTINATION ${INSTALL_CONFIGDIR})
