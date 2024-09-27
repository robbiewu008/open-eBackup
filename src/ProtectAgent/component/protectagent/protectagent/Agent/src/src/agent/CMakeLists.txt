if(LLT)
    add_library(rdagentobj
        OBJECT
        Authentication.cpp
        CheckCertValidity.cpp
        CheckConnectStatus.cpp
        Communication.cpp
        FTExceptionHandle.cpp
        TaskDispatchWorker.cpp
        TaskPool.cpp
        TaskProtectWorker.cpp
        TaskVssWorker.cpp
        TaskWorker.cpp
    )

    target_link_libraries(rdagentobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
else()
    aux_source_directory(. SRC_LIST)
    add_library(rdagentobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(rdagentobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
endif()

