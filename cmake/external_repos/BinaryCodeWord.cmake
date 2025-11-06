# BinaryCodeWord external repository configuration
# variables (overrideable via -D on the command line)
set(BINARY_CODE_WORD_GIT_URL "https://github.com/nathanjrussell/BinaryCodeWord.git" CACHE STRING "Git URL for BinaryCodeWord")
set(BINARY_CODE_WORD_FETCH_TAG "2.9.0" CACHE STRING "Tag to fetch if BinaryCodeWord not found")

find_package(BinaryCodeWord ${BINARY_CODE_WORD_FETCH_TAG} QUIET)

# fallback: fetch and add the project if not found
if(NOT BinaryCodeWord_FOUND)
    include(FetchContent)
    message(STATUS "BinaryCodeWord not found; fetching from ${BINARY_CODE_WORD_GIT_URL} tag ${BINARY_CODE_WORD_FETCH_TAG}")
    FetchContent_Declare(
            BinaryCodeWordProj
            GIT_REPOSITORY ${BINARY_CODE_WORD_GIT_URL}
            GIT_TAG        ${BINARY_CODE_WORD_FETCH_TAG}
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(BinaryCodeWordProj)
endif()

# File: `cmake/versions.cmake`


