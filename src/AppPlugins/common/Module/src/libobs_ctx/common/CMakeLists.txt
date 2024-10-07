# 递归添加当前目录下所有的cpp文件
file(GLOB_RECURSE obs_common_srcs *.cpp)

add_library(obs_common_obj
    OBJECT
    ${obs_common_srcs}
)

target_include_directories(obs_common_obj
    PUBLIC
    ${STORAGE_COMMON_INC}

    ${PLATFORM}/ALiCloudSDK_rel/include
    ${OPEN_SRC_PATH}/esdk_rel/include
)

target_link_directories(obs_common_obj
    PUBLIC
    ${PLATFORM}/ALiCloudSDK_rel/lib
    ${OPEN_SRC_PATH}/esdk_rel/lib
)

target_link_libraries(obs_common_obj
    PUBLIC
    alibabacloud-oss-cpp-sdk
    eSDKOBS
)