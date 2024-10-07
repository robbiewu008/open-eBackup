# 递归添加当前目录下所有的cpp文件
file(GLOB_RECURSE alicloud_srcs *.cpp)

add_library(alicloud_obj
    OBJECT
    ${alicloud_srcs}
)

target_include_directories(alicloud_obj
    PUBLIC
    ${STORAGE_COMMON_INC}

    ${PLATFORM}/ALiCloudSDK_rel/include
)

target_link_directories(alicloud_obj
    PUBLIC
    ${PLATFORM}/ALiCloudSDK_rel/lib
)

target_link_libraries(alicloud_obj
    PUBLIC
    alibabacloud-oss-cpp-sdk
)