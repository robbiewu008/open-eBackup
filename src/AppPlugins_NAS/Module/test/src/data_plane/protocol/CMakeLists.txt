MESSAGE("Compiling data_plane_protocol code, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE data_plane_protocol_test_srcs *.cpp)

add_library(data_plane_protocol_test_obj
    OBJECT
    ${data_plane_protocol_test_srcs}
)

target_include_directories(data_plane_protocol_test_obj
    PUBLIC
    ${GOOGLE_TEST_PATH}/googlemock/include
    ${GOOGLE_TEST_PATH}/googletest/include
    ${MODULE_ROOT_PATH}/src/data_plane/protocol/include
)

target_link_directories(data_plane_protocol_test_obj
    PUBLIC
    ${GOOGLE_TEST_PATH}/lib
)

target_link_libraries(data_plane_protocol_test_obj
    PUBLIC
    gtest
    data_plane_util
)