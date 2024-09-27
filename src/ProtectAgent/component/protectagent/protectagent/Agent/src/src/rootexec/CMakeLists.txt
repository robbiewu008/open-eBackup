if(LLT)
    add_library(rootexecobj
        OBJECT
        SystemCall.cpp
    )

    target_link_libraries(rootexecobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
else()
    aux_source_directory(. SRC_LIST)
    add_library(rootexecobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(rootexecobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
endif()

