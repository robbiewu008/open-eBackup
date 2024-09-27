MESSAGE("Compiling job manager, PATH is ${CMAKE_CURRENT_SOURCE_DIR}")

SET(INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../inc)

# 递归添加job_mgr目录下所有的cpp文件
file(GLOB_RECURSE job_mgr_srcs *.cpp)

if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "AIX")
    add_library(job_mgr_obj
        SHARED
        ${job_mgr_srcs}
    )
else()
    add_library(job_mgr_obj
        OBJECT
        ${job_mgr_srcs}
    )
endif()

target_include_directories(job_mgr_obj
    PUBLIC
    ${INC_PATH}/job_mgr
    ${INC_PATH}
)

target_link_libraries(job_mgr_obj
    PUBLIC
    basic_job
)
