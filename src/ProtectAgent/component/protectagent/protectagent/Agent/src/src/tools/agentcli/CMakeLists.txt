
if(LLT)
    add_library(agentcliobj
        OBJECT
        AddController.cpp
        ChangeIP.cpp
        ChgCrtPwd.cpp
        ChgHostSN.cpp
        ChgNgxPwd.cpp
        ChgSnmp.cpp
        CollectLog.cpp
        RegExtMk.cpp
        RegisterHost.cpp
        ShowStatus.cpp
        StartNginx.cpp
        TestHost.cpp
        UnixTimeStamp.cpp
        MergeConfFiles.cpp
        UpgradeJsonConf.cpp
    )

    target_link_libraries(agentcliobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
else()
    aux_source_directory(. SRC_LIST)
    add_library(agentcliobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(agentcliobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
endif()