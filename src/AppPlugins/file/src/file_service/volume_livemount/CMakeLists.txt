add_library(volume_livemount_obj
    OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/VolumeLivemount.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LinuxVolumeLivemount.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VolumeCancelLivemount.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LinuxVolumeCancelLivemount.cpp
)
 
target_include_directories(volume_livemount_obj
    PRIVATE
    ${PLUGIN_FRAMEWORK_PATH}/inc
    ${PLUGIN_FRAMEWORK_PATH}/inc/client
    ${PLUGIN_FRAMEWORK_PATH}/inc/common
    ${PLUGIN_FRAMEWORK_PATH}/inc/job/
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc
    ${PLUGIN_FRAMEWORK_PATH}/inc/rpc/certificateservice
    ${PLUGIN_SRC_PATH}
    ${PLUGIN_SRC_PATH}/common
    ${PLUGIN_SRC_PATH}/common/constant
    ${PLUGIN_SRC_PATH}/common/volume
    ${PLUGIN_SRC_PATH}/utils
)
 
target_link_libraries(volume_livemount_obj
    PRIVATE
    volume_obj
    common_dep
)