if(LINUX_COMPILE)
    add_subdirectory(certificateservice)
    add_subdirectory(messageservice)
    add_subdirectory(servicefactory)
    add_subdirectory(services)
    add_subdirectory(thriftservice)
    add_subdirectory(timerservice)

    add_library(ebkservicecenter
        STATIC
    )

    target_link_libraries(ebkservicecenter
        PRIVATE
        certificateserviceobj
        messageserviceobj
        servicefactoryobj
        servicesDeviceobj
        servicesJobserviceobj
        thriftServiceobj
        serviceTimerServiceobj
    )
else()
    add_library(ebkservicecenter
        INTERFACE
    )
endif()