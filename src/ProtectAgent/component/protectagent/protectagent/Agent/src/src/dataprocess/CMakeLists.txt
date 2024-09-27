if(LINUX_COMPILE)
    if (CMAKE_SYSTEM_NAME MATCHES "AIX" OR CMAKE_SYSTEM_NAME MATCHES "SunOS")
        add_subdirectory(datamessage)
    else()
        add_subdirectory(dataconfig)
        add_subdirectory(datamessage)
        add_subdirectory(datapath)
        add_subdirectory(datareadwrite)
        add_subdirectory(ioscheduler)
        add_subdirectory(jobqosmanager)
        add_subdirectory(vmwarenative)

        add_library(ebkdataprocess
            STATIC
        )

        target_link_libraries(ebkdataprocess
            PRIVATE
            dataconfigobj
            datamessageobj
            datapathobj
            datareadwriteobj
            ioschedulerobj
            jobqosmanagerobj
            dpvmwarenativeobj
        )
    endif()
else()
    add_subdirectory(datamessage)
endif()