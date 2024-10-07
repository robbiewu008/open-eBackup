MESSAGE("Compiling common, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/global.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../build/cmake/path.cmake)
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
SET(MODULE_ROOT_PATH ${MODULE_PATH}/..)

add_library(utility
    SHARED
    StackTracerForSignal.cpp)

target_include_directories(utility
    PUBLIC
    ${MODULE_PATH}
    ${PLATFORM}/SecureCLib_rel/include
)

target_link_directories(utility
    PUBLIC
    ${PLATFORM}/SecureCLib_rel/lib
)

target_link_libraries(utility
    PUBLIC
    securec
)
