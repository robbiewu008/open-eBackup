add_library(testTaskManagerobj
    OBJECT
    AppProtectJobHandlerTest.cpp
    PluginMainJobTest.cpp
    TaskContextTest.cpp
    TaskManagerTest.cpp
    TaskRedoFuncContainerTest.cpp
    TaskStepLinkTargetTest.cpp
    TaskStepPrepareNasMediaTest.cpp
    TaskStepPreSufScriptTest.cpp
    TaskStepScanDiskTest.cpp
    TaskStepScriptTest.cpp
    TaskStepTest.cpp
    TaskTest.cpp
#    externaljob/ExternalJobTest.cpp
    externaljob/JobStateActionTest.cpp
    externaljob/JobStateDBTest.cpp
    externaljob/PluginSubPrepJobTest.cpp
    # filter/DefaultRemoteHostFilterTest.cpp
)

target_link_libraries(testTaskManagerobj
    PRIVATE
    safe_cmplib
    agent_platform
)
