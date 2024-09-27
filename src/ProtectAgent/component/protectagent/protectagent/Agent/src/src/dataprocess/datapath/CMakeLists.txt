if(LLT)
    add_library(datapathobj
        OBJECT
        CdpDataPath.cpp
        DataPath.cpp
        HanderThread.cpp
        VMwareNativeDataPathImpl.cpp
        VMwareNativeDataPathProcess.cpp
        AsioDataMover.cpp
    )

    target_link_libraries(datapathobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
else()
    aux_source_directory(. SRC_LIST)
    add_library(datapathobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(datapathobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
endif()