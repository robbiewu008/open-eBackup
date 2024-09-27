cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(AgentLLT)

add_subdirectory(src)

# AgentHostTest
add_executable(AgentHostTest)
target_link_libraries(AgentHostTest
    testHostobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    taskmanagerobj
    tcpobj
    plugininterfaceobj
    pluginfxobj
    vmfsobj
    vmfs6obj    

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentPluginfxTest
add_executable(AgentPluginfxTest)
target_link_libraries(AgentPluginfxTest
    testPluginfxobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    taskmanagerobj
    tcpobj
    plugininterfaceobj
    pluginfxobj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentDeviceTest
add_executable(AgentDeviceTest)
target_link_libraries(AgentDeviceTest
    testDeviceobj
    testBaseLib
    agentOpenSrcLib
    deviceobj
    hostobj
    tcpobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentCommonTest
add_executable(AgentCommonTest)
target_link_libraries(AgentCommonTest
    testCommonobj
    testBaseLib
    agentOpenSrcLib

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentAlarmTest
add_executable(AgentAlarmTest)
target_link_libraries(AgentAlarmTest
    testAlarmobj
    testBaseLib
    agentOpenSrcLib
    tcpobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentArrayTest
add_executable(AgentArrayTest)
target_link_libraries(AgentArrayTest
    testArrayobj
    testBaseLib
    agentOpenSrcLib
    tcpobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentAgentTest
add_executable(AgentAgentTest)
target_link_libraries(AgentAgentTest
    testAgentobj
    testCunitpubobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpobj
    rdagentobj
    pluginfxobj
    datamessageobj
    pluginobj
    plugininterfaceobj
    taskmanagerobj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentDataprocessTest
add_executable(AgentDataprocessTest)
target_link_libraries(AgentDataprocessTest
    testDataprocessobj
    testBaseLib
    agentOpenSrcLib
    tcpobj

    ebkdataprocess

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentToolsTest
add_executable(AgentToolsTest)
target_link_libraries(AgentToolsTest
    testToolsMonitorobj
    testCunitpubobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpobj
    agentcliobj
    monitorobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentRootExecTest
add_executable(AgentRootExecTest)
target_link_libraries(AgentRootExecTest
    testRootexecobj
    testCunitpubobj
    testBaseLib
    agentOpenSrcLib
    tcpobj
    rootexecobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentPluginsTest
add_executable(AgentPluginsTest)
target_link_libraries(AgentPluginsTest
    testPluginobj
    testCunitpubobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpobj
    rootexecobj
    pluginobj
    datamessageobj
    pluginfxobj
    plugininterfaceobj
    taskmanagerobj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentPluginsClusterTest
add_executable(AgentPluginsClusterTest)
target_link_libraries(AgentPluginsClusterTest
    testPluginsClusterobj
    testCunitpubobj
    testBaseLib
    agentOpenSrcLib
    tcpobj
    pluginobj
    clusterobj
    pluginsClusterobj
    datamessageobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentPluginsDeviceTest
add_executable(AgentPluginsDeviceTest)
target_link_libraries(AgentPluginsDeviceTest
    testPluginsDeviceobj
    testCunitpubobj
    testBaseLib
    agentOpenSrcLib
    tcpobj
    pluginobj
    deviceobj
    pluginsDeviceobj
    datamessageobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentPluginsHostTest
add_executable(AgentPluginsHostTest)
target_link_libraries(AgentPluginsHostTest
    testPluginsHostobj
    testCunitpubobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpobj
    pluginobj
    pluginsHostobj
    pluginfxobj
    plugininterfaceobj
    rdagentobj
    datamessageobj
    taskmanagerobj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentPluginsOracletest --- Ã»ÓÐtcmalloc
add_executable(AgentPluginsOracletest)
target_link_libraries(AgentPluginsOracletest
    testPluginsOracleobj
    testCunitpubobj
    testBaseLib
    agentOpenSrcLib
    tcpobj
    pluginobj
    oracleobj
    pluginsOracleobj
    datamessageobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)


# AgentPluginsOracleNativetest
add_executable(AgentPluginsOracleNativetest)
target_link_libraries(AgentPluginsOracleNativetest
    testPluginsOracleNativeobj
    testCunitpubobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpobj
    pluginobj
    taskmanagerobj
    oracleobj
    datamessageobj
    deviceobj
    pluginfxobj
    plugininterfaceobj
    oraclenativeobj
    pluginsOracleNativeobj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentRestTest
add_executable(AgentRestTest)
target_link_libraries(AgentRestTest
    testRestobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpobj
    pluginobj
    pluginfxobj
    taskmanagerobj
    plugininterfaceobj
    datamessageobj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentTcpTest
add_executable(AgentTcpTest)
target_link_libraries(AgentTcpTest
    testTcpobj
    testBaseLib
    agentOpenSrcLib
    tcpobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentCurlHttpClientTest
add_executable(AgentCurlHttpClientTest)
target_link_libraries(AgentCurlHttpClientTest
    testCurlclientobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpobj
    rdagentobj
    pluginfxobj
    plugininterfaceobj
    taskmanagerobj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentAppOracleTest
add_executable(AgentAppOracleTest)
target_link_libraries(AgentAppOracleTest
    testOracleobj
    testBaseLib
    agentOpenSrcLib
    tcpobj
    oracleobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentAppOracleNativeTest
add_executable(AgentAppOracleNativeTest)
target_link_libraries(AgentAppOracleNativeTest
    testOracleNativeobj
    testCunitpubobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpobj
    pluginobj
    oraclenativeobj
    testMainobj
    taskmanagerobj
    oracleobj
    datamessageobj
    deviceobj
    plugininterfaceobj
    pluginfxobj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentTaskmanagerTest
add_executable(AgentTaskmanagerTest)
target_link_libraries(AgentTaskmanagerTest
    testTaskManagerobj
    testClusterobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpobj
    taskmanagerobj
    plugininterfaceobj
    rdagentobj
    pluginfxobj
    clusterobj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentXBSAServerTest
add_executable(AgentXBSAServerTest)
target_link_libraries(AgentXBSAServerTest
    testXBSAServerobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpsslobj
    clusterobj
    xbsaserverobj
    xbsacomobj
    DwsDppClientobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentXbsaTest
add_executable(AgentXbsaTest)
target_link_libraries(AgentXbsaTest
    testXbsaobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    taskmanagerobj
    tcpobj
    rdagentobj
    xbsacomobj
    plugininterfaceobj
    pluginfxobj
    xbsaclientobj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentDwsTest
add_executable(AgentDwsTest)
target_link_libraries(AgentDwsTest
    testDwsobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpsslobj
    pluginobj
    datamessageobj
    xbsaserverobj
    xbsacomobj
    DwsDppClientobj
    pluginsDwsobj

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentServiceCenterTest
add_executable(AgentServiceCenterTest)
target_link_libraries(AgentServiceCenterTest
    testCertificateServiceobj
    testMessageServiceobj
    testServiceFactoryobj
    testServicesDeviceobj
    testServicesJobServiceobj
    testThriftServiceobj
    testTimerServiceobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpsslobj
    plugininterfaceobj
    pluginfxobj
    taskmanagerobj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

# AgentProtectPluginTest
add_executable(AgentProtectPluginTest)
target_link_libraries(AgentProtectPluginTest
    testPluginInterfaceobj
    testAppprotectobj
    testBaseLib
    agentOpenSrcLib
    hostobj
    tcpobj
    pluginfxobj
    appprotectobj
    plugininterfaceobj
    taskmanagerobj
    pluginsAppprotectobj
    datamessageobj
    pluginobj
    vmfsobj
    vmfs6obj

    ebkservicecenter

    # system lib
    pthread
    rt
    dl

    # compile option
    safe_cmpexec
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/test/stubTest/bin)