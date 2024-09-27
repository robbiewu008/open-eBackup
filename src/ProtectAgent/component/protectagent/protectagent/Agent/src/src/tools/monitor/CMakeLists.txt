if(LLT)
    add_library(monitorobj
        OBJECT
        AbnormalHandler.cpp
    )

    target_link_libraries(monitorobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
else()
    aux_source_directory(. SRC_LIST)
    add_library(monitorobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(monitorobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
endif()
