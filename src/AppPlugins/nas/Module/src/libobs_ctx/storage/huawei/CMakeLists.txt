# 递归添加当前目录下所有的cpp文件
file(GLOB_RECURSE hcs_srcs *.cpp)

add_library(hcs_obj
    OBJECT
    ${hcs_srcs}
)

target_include_directories(hcs_obj
    PUBLIC
    ${STORAGE_COMMON_INC}

    ${OPEN_SRC_PATH}/esdk_rel/include
)

target_link_directories(hcs_obj
    PUBLIC
    ${OPEN_SRC_PATH}/esdk_rel/lib
)

target_link_libraries(hcs_obj
    PUBLIC
    eSDKOBS
)