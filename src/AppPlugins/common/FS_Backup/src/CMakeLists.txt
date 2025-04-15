add_subdirectory(common)
add_subdirectory(engines)
add_subdirectory(model)
add_subdirectory(service)

# UNIX不支持卷备份
if (VOLUME AND "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    message("add subdirectory volume")
    add_subdirectory(volume)
    add_subdirectory(bare_metal_recovery)
endif ()