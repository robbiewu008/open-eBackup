add_library(agent_platform
    INTERFACE
    IMPORTED
    GLOBAL)

if(NOT WIN32)
    target_include_directories(agent_platform
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../obj/libsecurec/include
        ${CMAKE_CURRENT_SOURCE_DIR}/kmc/include
        ${CMAKE_CURRENT_SOURCE_DIR}/kmc
    )

    target_link_directories(agent_platform
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../obj/libsecurec/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/kmc/lib
    )
endif()
