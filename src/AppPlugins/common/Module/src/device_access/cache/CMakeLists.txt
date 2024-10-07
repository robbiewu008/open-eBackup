
file(GLOB_RECURSE session_srcs *.cpp)

add_library(session_device_obj
    OBJECT
    ${session_srcs}
)

target_include_directories(session_device_obj
    PUBLIC
    ${MODULE_SRC_PATH}
    ${PLATFORM}/SecureCLib_rel/include
    ${OPEN_SRC}/boost_rel/include
    ${OPEN_SRC}/jsoncpp_rel/include
)

