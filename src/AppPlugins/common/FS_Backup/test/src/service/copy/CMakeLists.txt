add_library(service_copy_obj
    OBJECT
    PosixCopyTest.cpp
)

target_include_directories(service_copy_obj
    PUBLIC
    ${MODULE_ROOT_PATH}/src
    ${PLATFORM}/securec/include
    ${DT_UTILS_PATH}/llt_stub
    ${BACKUP_SRC_PATH}

    ${BACKUP_SRC_PATH}/common
    ${BACKUP_SRC_PATH}/model
    ${OPEN_SRC_PATH}/lnfs_rel/include
    ${OPEN_SRC_PATH}/lsmb2_rel/include
    ${MODULE_ROOT_PATH}/src/common
    ${MODULE_ROOT_PATH}/src/libsmb_ctx
    ${MODULE_ROOT_PATH}/src/log
    ${MODULE_ROOT_PATH}/src
    ${MODULE_ROOT_PATH}/src/parser/
    ${MODULE_ROOT_PATH}/third_open_src/boost_rel/include
    ${BACKUP_SRC_PATH}/engines/libsmb
    ${BACKUP_SRC_PATH}/service/copy
    ${BACKUP_SRC_PATH}/service
)

target_link_libraries(service_copy_obj

)