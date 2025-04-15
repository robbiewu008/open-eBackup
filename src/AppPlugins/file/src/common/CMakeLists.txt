
# 添加编译的子目录
add_subdirectory(application)
add_subdirectory(host)
if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    add_subdirectory(volume)
endif()