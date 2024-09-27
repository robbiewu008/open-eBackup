if(LLT)
    message( STATUS "LLT mode, not generate scriptsign object.")
else()
    aux_source_directory(. SRC_LIST)
    add_library(scriptsignobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(scriptsignobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
endif()