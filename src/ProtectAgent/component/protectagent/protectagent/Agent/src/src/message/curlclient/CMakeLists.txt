set(CMAKE_CXX_STANDARD 11)
if(LINUX_COMPILE)
    aux_source_directory(. SRC_LIST)
    add_library(curlclientobj
        OBJECT
        ${SRC_LIST}
    )

    target_link_libraries(curlclientobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
else()
    add_library(curlclientobj
        OBJECT
        CurlHttpClient.cpp
        RestClientCommon.cpp
        SSLHandle.cpp
    )

    target_link_libraries(curlclientobj
        PRIVATE
        safe_cmplib
        agent_platform
    )
endif()