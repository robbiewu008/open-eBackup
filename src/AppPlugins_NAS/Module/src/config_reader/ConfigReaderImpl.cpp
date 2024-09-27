/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include <securec.h>
#include <thread>
#include <iostream>
#include <mutex>
#include <fstream>
#include <map>
#include <sstream>
#include "config_reader/ConfigIniReaderImpl.h"
#include "config_reader/ConfigIniReader.h"
#include "config_reader/ConfigIniValue.h"
#include "log/Log.h"
#include "common/Path.h"
#include "common/CTime.h"
#include "common/File.h"
#include "common/MpString.h"

#ifdef __WINDOWS__
// need the following include for GetModuleFileName()
#include <Windows.h>
#endif

using namespace Module;
using namespace std;

namespace {
    const string FILE_SERVER_SSL_CONFIG = "fileServerSslEnabled";
    const string PLUGIN_CONF_PATH = "hcpconf.ini";
    const string PM_LOG_LEVEL_PATH = "/opt/common-conf/loglevel";
    const int DELAY_SECOND = 1;
}

#define GET_VALUE(T, getFuncName, initValue)                                                                           \
    T ConfigReaderImpl::getFuncName(const string& sectionName, const string& keyName, bool logFlag) const              \
    {                                                                                                                  \
        T rtValue = initValue;                                                                                         \
        ConfInfoMap::const_iterator ite = m_valueInfo.find(Configkey(sectionName, keyName));                           \
        if (ite != m_valueInfo.end()) {                                                                                \
            if (ite->second != NULL) {                                                                                 \
                ite->second->getValue(rtValue);                                                                        \
            }                                                                                                          \
        } else {                                                                                                       \
            if (logFlag) {                                                                                             \
                HCP_Logger_noid(CRIT, MODULE_NAME) << "Reading non-existed configuration item: {" << sectionName       \
                                                   << "," << keyName << "}." << HCPENDLOG;                             \
            }                                                                                                          \
        }                                                                                                              \
        return rtValue;                                                                                                \
    }

namespace Module {

ConfigReaderImpl* ConfigReaderImpl::m_pInstance;
//lint -e708      //union initialization
#ifdef WIN32
thread_lock_t ConfigReaderImpl::m_instMutext;
thread_lock_t ConfigReaderImpl::m_configMutext;
thread_lock_t ConfigReaderImpl::m_configFileMutext;
#else
thread_lock_t ConfigReaderImpl::m_instMutext = PTHREAD_MUTEX_INITIALIZER;
thread_lock_t ConfigReaderImpl::m_configMutext = PTHREAD_MUTEX_INITIALIZER;
thread_lock_t ConfigReaderImpl::m_configFileMutext = PTHREAD_MUTEX_INITIALIZER;
#endif
//lint +e708
const char* DEFAULT_CONFIG_PATH = "/opt/huawei-data-protection/ebackup/conf";

// This class is implememted as a single instance
ConfigReaderImpl* ConfigReaderImpl::instance()
{
    if (NULL == m_pInstance) {
#ifdef WIN32
        InitializeCriticalSection(&ConfigReaderImpl::m_instMutext);
        InitializeCriticalSection(&ConfigReaderImpl::m_configMutext);
        InitializeCriticalSection(&ConfigReaderImpl::m_configFileMutext);
#endif
        CThreadAutoLock lock(&m_instMutext);
        if (NULL == m_pInstance) {
            m_pInstance = new (nothrow) ConfigReaderImpl();
            if (NULL == m_pInstance) {
                HCP_Logger_noid(ERR, MODULE_NAME) << "new ConfigReaderImpl failed" << HCPENDLOG;
            }
        }
    }
    return m_pInstance;
}

void ConfigReaderImpl::destroy()
{
    CThreadAutoLock lock(&m_instMutext);
    if (m_pInstance != NULL) {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

ConfigReaderImpl::ConfigReaderImpl()
{
    try {
        m_timer.os_id = 0;  // if new failed, the pointer will be invalid, so there must init it.
        m_running = true;
        // 插件默认不从PM的文件读取配置方式
        m_readLoglevelFromPM = false;
        m_pluginConfFile = make_pair<string, time_t>("/opt/common-conf/loglevel", 0);
        string agentConfFile = CPath::GetInstance().GetAgentConfFilePath("agent_cfg.xml");
        m_agentConfFile = make_pair<string, time_t>(agentConfFile.c_str(), 0);
        logLevelAdnValue["DEBUG"] = "0";
        logLevelAdnValue["INFO"] = "1";
        logLevelAdnValue["WARN"] = "2";
        logLevelAdnValue["ERROR"] = "3";
        logLevelAdnValue["CRITICAL"] = "4";
        InitConfigInfos();

        loadConfigFiles();

        // To guarantee the value is set while call getXX
        // Because call refresh() will wait a delay time and make the time of start service longer, call
        // refresh(getConfigFiles()).
        refresh(getConfigFiles());
        // Start timer
        int iRet = Module::CMpThread::Create(&m_timer, ConfigReaderImpl::TimerThreadProc, this);
        if (iRet != SUCCESS) {
            HCP_Logger_noid(INFO, MODULE_NAME) << "Create receive thread failed! iRet = " << iRet << HCPENDLOG;
            return;
        }
    } catch (exception& e) {
        HCP_Logger_noid(ERR, MODULE_NAME) << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
        clear();
    }
#ifdef WIN32
    InitializeCriticalSection(&ConfigReaderImpl::m_instMutext);
    InitializeCriticalSection(&ConfigReaderImpl::m_configMutext);
    InitializeCriticalSection(&ConfigReaderImpl::m_configFileMutext);
#endif
}

void ConfigReaderImpl::clear()
{
    m_running = false;
    if (0 != m_timer.os_id) {
        (void)CMpThread::WaitForEnd(&m_timer, NULL);
        m_timer.os_id = 0;
    }

    for (ConfInfoMap::iterator ite = m_valueInfo.begin(); ite != m_valueInfo.end(); ++ite) {
        if (ite->second != NULL) {
            delete ite->second;
            ite->second = NULL;
        }
    }
    m_valueInfo.clear();
}

ConfigReaderImpl::~ConfigReaderImpl()
{
    clear();
}

GET_VALUE(int, getInt, -1)
GET_VALUE(string, getString, "")

void ConfigReaderImpl::InitConfigInfosForGeneralForOther()
{
    putIntConfigInfo("General", "ReadConfInterval", 1, 30, 10);
    putIntConfigInfo("General", "MachineRole", 0, 3, 0);
    putIntConfigInfo("General", "CreateBitmapTimeoutSec", 5, 2592000, 604800);
    putIntConfigInfo("General", "AlarmDelayInterval", 0, 3600, 180);
    putIntConfigInfo("General", "SqliteSync", 0, 2, 2);
    putIntConfigInfo("General", "HasRegistedToGov", 0, 1, 0);
    putIntConfigInfo("General", "SetCPUAffinityEnable", 0, 1, 1);
    putIntConfigInfo("General", "MaxCPUUsagePercent", 0, 2147483647, 100);
    putIntConfigInfo("General", "CapacityBalanceModeDistAlg", 0, 4, 1);
    putIntConfigInfo("General", "FCGI_HANDLER_SIZE", 100, 100000, 1024);
    putIntConfigInfo("General", "REQ_MAX_SIZE", 1, 100000, 512);
    putIntConfigInfo("General", "GrayTrace", 0, 1, 0);
    putIntConfigInfo("General", "AllowParamCheck", 0, 1, 1);
    // allow DlibOpen in mode DFLG_GLOBAL, 0:yes, 1:no
    putIntConfigInfo("General", "AllowDlOpenGlobal", 0, 1, 1);

    putStringConfigInfo("General", "BackupStoragePlane", "127.0.0.1|00:00:00:00:00:00");
    putStringConfigInfo("General", "RegisterName", "eBackup");
    putStringConfigInfo("General", "ProcessCPUAffinityMask", "F");
    putStringConfigInfo("General", "CPUCGroupName", "cloudbackup/ebk_");
    putStringConfigInfo("General", "MemoryCGroupName", "cloudbackup/ebk_");
    putStringConfigInfo("General", "MaxMemoryUsage", "");
    putStringConfigInfo("General", "PrimaryAndStandbyIP", "127.0.0.1");
    putStringConfigInfo("General", "CurrentLeaderIP", "127.0.0.1");
    putStringConfigInfo("General", "NetworkType", "ipv4");
    putStringConfigInfo("General", "MicroServiceName", "");
}

void ConfigReaderImpl::InitConfigInfosForGeneral()
{
    // log config
    putIntConfigInfo("General", "LogLevel", 0, 5, 2);
    putIntConfigInfo("General", "LogFlushTime", 1, 120, 1);
    putIntConfigInfo("General", "LogLimitIntervalTime", 0, 100000, 0);
    putIntConfigInfo("General", "LogLimitFrequency", 1, 10000, 2);
    putIntConfigInfo("General", "LogCount", 0, 100, 3);
    putIntConfigInfo("General", "LogMaxSize", 0, 200, 50);

    putIntConfigInfo("General", "OBSS3ipv6PortHttp", 1024, 65535, 5080);
    putIntConfigInfo("General", "OBSS3ipv6PortHttps", 1024, 65535, 5443);

    // HCP config
    putStringConfigInfo("General", "HCPManagementPlane", "127.0.0.1|00:00:00:00:00:00");
    putStringConfigInfo("General", "HCPInternalPlane", "127.0.0.1|00:00:00:00:00:00");

    // ProtectedEnvironment config
    putIntConfigInfo("General", "FileSystemSetVariableSegment", 0, 128, 8);
    putIntConfigInfo("General", "FileSystemSetDistAlg", 0, 4, 4);
    putStringConfigInfo("General", "ProtectedEnvironmentManagementPlane", "127.0.0.1|00:00:00:00:00:00");
    putStringConfigInfo("General", "ProtectedEnvironmentStoragePlane", "127.0.0.1|00:00:00:00:00:00");

    // http config
    putIntConfigInfo("General", "HTTP_BUSY_REPEAT_INTERVAL", 1, 6000, 2);
    putIntConfigInfo("General", "HTTP_BUSY_REPEAT_TIME", 1, (3600 * 24 * 365), (3600 * 24));

    // port config
    putIntConfigInfo("General", "Port", 0, 65535, 8091); // 0-min,65535-max,8091-default

    // other config
    InitConfigInfosForGeneralForOther();
}

void ConfigReaderImpl::InitConfigInfosForAdminNode()
{
    putIntConfigInfo("AdminNode", "HeartBeatServerPort", 1024, 65535, 5569);
}

void ConfigReaderImpl::InitConfigInfosForBackupNode()
{
    putIntConfigInfo("BackupNode", "S3Timeout", 1, 600, 300);
    putIntConfigInfo("BackupNode", "S3RetryTimes", 0, 3, 3);
    putIntConfigInfo("BackupNode", "S3RetryTimeout", 1, 60, 60);
    putIntConfigInfo("BackupNode", "NeedInitS3", 0, 1, 1);
    putIntConfigInfo("BackupNode", "S3ConnectionTimeOut", 1, 3600, 180);
    putIntConfigInfo("BackupNode", "S3BucketFullControl", 0, 1, 1);
    putIntConfigInfo("BackupNode", "S3URLStyle", 0, 1, 0);

    putIntConfigInfo("BackupNode", "restoreTryTimes", 1, 1000, 3);
    putIntConfigInfo("BackupNode", "restoreRewriteInteval", 1, 1000, 30);
    putIntConfigInfo("BackupNode", "rereadTryTimes", 1, 1000, 3);
    putIntConfigInfo("BackupNode", "backupRereadInteval", 1, 1000, 30);
    putIntConfigInfo("BackupNode", "HeartBeatDefaultBindPort",  1024, 65535, 5570);
    putIntConfigInfo("BackupNode", "TestNetTimeOut", 1, 172800, 15);
    putIntConfigInfo("BackupNode", "NoMatchCertWarning", 0, 1, 1);
    putIntConfigInfo("BackupNode", "RetryHealthStatusTimes", 0, 10, 3);
    putIntConfigInfo("BackupNode", "RetryHealthStatusWaitTime", 0, 60, 30);
    putIntConfigInfo("BackupNode", "DosRequestInterval", 1, 60, 1);
    putIntConfigInfo("BackupNode", "DosRequestCount", 1, 1000, 200);
    putIntConfigInfo("BackupNode", "CheckLocalChainDBIntegrity", 0, 1, 1);

    putStringConfigInfo("BackupNode", "CurrentLeaderIP", "127.0.0.1");
    putStringConfigInfo("BackupNode", "IAMUser", "");
    putStringConfigInfo("BackupNode", "IAMPasswd", "");
    putStringConfigInfo("BackupNode", "NotRetryErrorCodeForDiskArray", "50331651,1077949061,1077987871,1077949081," \
        "1077949070,1077949071,1077937889,1077937880,1073745412,1077937498,1077937500,1073804552,1077936861," \
        "1073804554,1077949004,1073846111,1073846112,1077951819,1077950342,1077949002,-20001,1077936867");
}

void ConfigReaderImpl::InitConfigInfoForMicroService()
{
    putIntConfigInfo("MicroService", "MKValidDays", 1, 36500, 1825);
    putIntConfigInfo("MicroService", "RunShellByBoost", 0, 2, 1);
    putIntConfigInfo("MicroService", "RunShellByBoost", 0, 2, 1);

    putStringConfigInfo("MicroService", "LoadbalanceAddress", "https://127.0.0.1:59000");
    putStringConfigInfo("MicroService", "IAMUser", "");    // only for current status,later should use security way
    putStringConfigInfo("MicroService", "IAMPasswd", "");  // only for current status,later should use security way
    putStringConfigInfo("MicroService", "HttpStatusCodesForRetry", "");
}

void ConfigReaderImpl::InitConfigInfoForDataBase()
{
    putIntConfigInfo("DataBase", "DBIntervalTime", 0, 100, 30);
    putIntConfigInfo("DataBase", "DBReconnectTimes", 0, 100, 30);
}

void ConfigReaderImpl::InitConfigInfoForWDS()
{
    putIntConfigInfo("DWSConfig", "SnapShotCnt", 0, 10000, 10000);
    putIntConfigInfo("DWSConfig", "SnapOverTime", 0, 30, 30);
    putIntConfigInfo("DWSConfig", "ThreadCnt", 0, 10, 10);
}

void ConfigReaderImpl::InitConfigInfoForHCS()
{
    putIntConfigInfo("HcsConfig", "CreateBitmapVolumeTimeOut", 300, 600, 7200);
    putIntConfigInfo("HcsConfig", "CreateSnapshotLimit", 1, 30, 10);
    putIntConfigInfo("HcsConfig", "CreateSnapshotApigwFailedRetry", 3, 10, 3);
}

void ConfigReaderImpl::InitConfigInfos()
{
    InitConfigInfosForGeneral();
    InitConfigInfosForAdminNode();
    InitConfigInfosForBackupNode();
    InitConfigInfoForMicroService();
    InitConfigInfoForDataBase();
    InitConfigInfoForWDS();
    InitConfigInfoForHCS();


    putIntConfigInfo("ProcessMonitor", "ProcessMonitorTimeInterval", 1, 300, 10);
    putIntConfigInfo("Cert", "NoMatchCertWarning", 0, 1, 1);
    putIntConfigInfo("Md5", "UploadWithMd5", 0, 1, 0);
    putIntConfigInfo("Vpp", "TCPListenPort", 1024, 65535, 31120);
    putIntConfigInfo("Vpp", "ISVPP", 0, 1, 0);
}

void ConfigReaderImpl::putIntConfigInfo(
    const string& sectionName, const string& keyName, int minValue, int maxValue, int defaultValue)
{
    m_valueInfo.insert(
        ConfInfoMap::value_type(Configkey(sectionName, keyName), new IntConfigValue(minValue, maxValue, defaultValue)));
}

void ConfigReaderImpl::putStringConfigInfo(
    const string& sectionName, const string& keyName, const string& defaultValue)
{
    m_valueInfo.insert(ConfInfoMap::value_type(Configkey(sectionName, keyName), new StringConfigValue(defaultValue)));
}

void ConfigReaderImpl::putIPConfigInfo(
    const string& sectionName, const string& keyName, const string& defaultValue)
{
    m_valueInfo.insert(ConfInfoMap::value_type(Configkey(sectionName, keyName), new IPStringConfigValue(defaultValue)));
}

vector<string> ConfigReaderImpl::getConfigFiles()
{
    CThreadAutoLock lock(&m_configFileMutext);  //lint !e830
    vector<string> files;
    for (size_t i = 0; i < m_confFileName.size(); i++) {
        files.push_back(m_confFileName[i].first);
    }
    return files;
}  //lint !e1788

void ConfigReaderImpl::loadConfigFiles()
{
    CThreadAutoLock lock(&m_configFileMutext);  //lint !e830

    if (!AddConfigFile("hcpconf.ini")) {
        HCP_Logger_noid(WARN, MODULE_NAME) << "Add config file hcpconf.ini failed" << HCPENDLOG;
    }

}

#ifdef WIN32
DWORD WINAPI ConfigReaderImpl::TimerThreadProc(void* pConfigReader)
#else
void* ConfigReaderImpl::TimerThreadProc(void* pConfigReader)
#endif
{
    if (NULL != pConfigReader) {
        ConfigReaderImpl* pThis = reinterpret_cast<ConfigReaderImpl*>(pConfigReader);
        pThis->ConfigReaderTimer();
    }
#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}
// Refresh value from conf/hcpconf.ini every 30 seconds.
void ConfigReaderImpl::ConfigReaderTimer()
{
    // Note only modify the value of ReadConfInterval while debug.
    int refreshInterval = getInt("General", "ReadConfInterval");
    // The thread is stopped by interrupt
    while (m_running) {
        CTime::DoSleep(refreshInterval * 1000);
        refresh();
    }
}

tinyxml2::XMLElement* ConfigReaderImpl::GetChildElement(tinyxml2::XMLElement* pParentElement, const string& strSection) const
{
    if (pParentElement == nullptr) {
        return nullptr;
    }

    tinyxml2::XMLElement* pCfgSec = pParentElement->FirstChildElement();
    if (pCfgSec  == nullptr) {
        return nullptr;
    }

    while (pCfgSec) {
        // Coverity&Fortify误报:FORTIFY.Null_Dereference
        const char* sectionName = pCfgSec->Value();
        if (sectionName == nullptr || *sectionName == 0) {
            pCfgSec = pCfgSec->NextSiblingElement();
            continue;
        }

        if (strcmp(sectionName, strSection.c_str()) == 0) {
            return pCfgSec;
        } else {
            pCfgSec = pCfgSec->NextSiblingElement();
        }
    }

    return nullptr;
}

string ConfigReaderImpl::GetLogLevelFromXml(string& xmlFilePath, const string& strSection) const
{
    string logLevel;
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xmlFilePath.c_str())) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "Load config xml file failed." << HCPENDLOG;
        return logLevel;
    }
    tinyxml2::XMLElement* rootElement = doc.RootElement();
    tinyxml2::XMLElement* pCfgSec = GetChildElement(rootElement, "System");
    if (pCfgSec == nullptr) {
        return logLevel;
    }
        tinyxml2::XMLElement* pChildItem = pCfgSec->FirstChildElement();
    if (pChildItem == nullptr) {
        return logLevel;
    }

    while (pChildItem) {
        // Coverity&Fortify误报:FORTIFY.Null_Dereference
        const char* nodeName = pChildItem->Value();
        if (nodeName == nullptr || *nodeName == 0) {
            pChildItem = pChildItem->NextSiblingElement();
            continue;
        }
        if (strcmp(nodeName, strSection.c_str()) == 0) {
            const tinyxml2::XMLAttribute* pAttr = pChildItem->FirstAttribute();
            logLevel = pAttr->Value();
            break;
        }
        pChildItem = pChildItem->NextSiblingElement();
    }
    return logLevel;
}

// Get BackupScene to discriminate inner or external
string ConfigReaderImpl::GetBackupSceneFromXml(const string& strSection) const
{
    string backupScene;
    tinyxml2::XMLDocument doc;
    DBGLOG("agentConfigPath:%s", m_agentConfFile.first.c_str());
    if (doc.LoadFile(m_agentConfFile.first.c_str())) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "Load config xml file failed." << HCPENDLOG;
        return backupScene;
    }
    tinyxml2::XMLElement* rootElement = doc.RootElement();
    tinyxml2::XMLElement* pCfgSec = GetChildElement(rootElement, "Backup");
    if (pCfgSec == nullptr) {
        return backupScene;
    }
    tinyxml2::XMLElement* pChildItem = pCfgSec->FirstChildElement();
    if (pChildItem == nullptr) {
        return backupScene;
    }

    while (pChildItem) {
        // Coverity&Fortify误报:FORTIFY.Null_Dereference
        const char* nodeName = pChildItem->Value();
        if (nodeName == nullptr || *nodeName == 0) {
            pChildItem = pChildItem->NextSiblingElement();
            continue;
        }
        if (strcmp(nodeName, strSection.c_str()) == 0) {
            const tinyxml2::XMLAttribute* pAttr = pChildItem->FirstAttribute();
            backupScene = pAttr->Value();
            break;
        }
        pChildItem = pChildItem->NextSiblingElement();
    }
    return backupScene;
}

// Refresh value from conf/hcpconf.ini
void ConfigReaderImpl::refresh()
{
    // 外置插件通过查看agent配置文件更改日志级别
    if (m_updateCnf) {
        if (!m_readLoglevelFromPM && isFileModified(m_agentConfFile)) {
            string logLevel = GetLogLevelFromXml(m_agentConfFile.first, "log_level");
            if (!UpdatePluginConfigFile(logLevel)) {
                HCP_Logger_noid(WARN, MODULE_NAME)
                    << "Failed in synchronizing the log level file of plugin." << HCPENDLOG;
            }
        }

        if (m_readLoglevelFromPM && isFileModified(m_pluginConfFile)) {
            HCP_Logger_noid(INFO, MODULE_NAME)
                    << "PM LOG LEVEL file has been modified." << HCPENDLOG;
            if (ModifyConfigurationFile(PM_LOG_LEVEL_PATH)) {
                HCP_Logger_noid(INFO, MODULE_NAME)
                    << "Succeeded to synchronize the log level file of plugin." << HCPENDLOG;
            } else {
                HCP_Logger_noid(ERR, MODULE_NAME)
                    << "Failed in synchronizing the log level file of plugin." << HCPENDLOG;
            }
        }
    }

    for (size_t i = 0; i < m_confFileName.size(); i++) {
        // Read file only when the file is modified
        if (isFileModified(m_confFileName[i])) {
            HCP_Logger_noid(INFO, MODULE_NAME)
                << "Find config file is modified. path " << m_confFileName[i].first << HCPENDLOG;
            CTime::DoSleep(DELAY_SECOND * 1000);
            refresh(getConfigFiles());
            break;
        }
    }
}

bool ConfigReaderImpl::ModifyConfigurationFile(string configPathToRead)
{
    string logLevel;
    if (!ReadPmLogLevel(configPathToRead, logLevel)) {
        return false;
    }
    if (!UpdatePluginConfigFile(logLevel)) {
        return false;
    }
    return true;
}

bool ConfigReaderImpl::ReadPmLogLevel(string configPathToRead, string &logLevel)
{
    ifstream streamToRead;
    streamToRead.open(configPathToRead.c_str(), ifstream::in);
    if (!streamToRead.is_open()) {
        HCP_Logger_noid(INFO, MODULE_NAME) << "To read configuration file failed, file is" <<
            configPathToRead << HCPENDLOG;
        return false;
    }
    getline(streamToRead, logLevel);
    HCP_Logger_noid(INFO, MODULE_NAME) << "logLevel that PM give plugin is: "<< logLevel << "." <<HCPENDLOG;
    streamToRead.close();
    return true;
}

bool ConfigReaderImpl::IsLogLevelValue(const string& logLevel) const
{
    for (auto it = logLevelAdnValue.begin(); it != logLevelAdnValue.end(); ++it) {
        if (it->second == logLevel) {
            return true;
        }
    }
    return false;
}

bool ConfigReaderImpl::UpdatePluginConfigFile(string logLevel)
{
    string logLevelValue;
    if (logLevelAdnValue.count(logLevel)) {
        logLevelValue = logLevelAdnValue[logLevel];
    } else if (IsLogLevelValue(logLevel)) {
        logLevelValue = logLevel;
    } else {
        HCP_Logger_noid(ERR, MODULE_NAME) << "Get logLevel failed, please check parameters. logLevel is "<<
            logLevel << HCPENDLOG;
        return false;
    }

    std::string pluginConfigFile = CPath::GetInstance().GetConfFilePath(PLUGIN_CONF_PATH);
    ifstream streamRead;
    streamRead.open(pluginConfigFile.c_str(), ios::in);
    if (!streamRead.is_open()) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "Write configuration file failed, file is" <<
            pluginConfigFile << HCPENDLOG;
        return false;
    }
    stringstream readbuf;
    readbuf << streamRead.rdbuf();
    streamRead.close();
    
    stringstream configFileContent;
    string cfgLine;
    while (getline(readbuf, cfgLine)) {
        if (cfgLine.find("LogLevel=") != string::npos) {
            configFileContent << "LogLevel=" + logLevelValue << endl;
            continue;
        }
        configFileContent << cfgLine << endl;
    }

    ofstream streamWrite;
    streamWrite.open(pluginConfigFile.c_str(), ios::out | ios::trunc);
    streamWrite << configFileContent.str();
    streamWrite.close();
    HCP_Logger_noid(INFO, MODULE_NAME) << "UpdatePluginConfigFile success" << HCPENDLOG;
    return true;
}

void ConfigReaderImpl::refresh(const vector<string>& confFileName)
{
    CThreadAutoLock lock(&m_configMutext);  //lint !e830

    vector<CIniFile*> iniFiles;
    int iRet = SUCCESS;
    //lint -e429
    for (size_t i = 0; i < confFileName.size(); i++) {
        CIniFile* pFile = new (nothrow) CIniFile();

        if (NULL != pFile) {
            iRet = pFile->Init(confFileName[i]);
            if (iRet != SUCCESS) {
                HCP_Logger_noid(ERR, MODULE_NAME)
                    << "Init config file failed. fullpath=" << confFileName[i] << HCPENDLOG;
                delete pFile;
                pFile = NULL;
                continue;
            }
            iniFiles.push_back(pFile);
        }
    }
    //lint +e429
    // Read and check and set values one by one
    for (ConfInfoMap::iterator iteValueInfo = m_valueInfo.begin(); iteValueInfo != m_valueInfo.end(); ++iteValueInfo) {
        const string& sectionName = iteValueInfo->first.sectionName;
        const string& keyName = iteValueInfo->first.keyName;
        if (iteValueInfo->second != NULL) {
            iteValueInfo->second->refreshValue(sectionName, keyName, iniFiles);
        }
    }

    vector<CIniFile*>::iterator it = iniFiles.begin();
    for (; it != iniFiles.end(); ++it) {
        delete (*it);
        (*it) = NULL;
    }

    // refresh log
    setLogConf();
}  //lint !e1788

void ConfigReaderImpl::setLogConf()
{
    // update log info
    int logLevel = getInt(string("General"), string("LogLevel"));
    int logCount = getInt(string("General"), string("LogCount"));
    int logMaxSize = getInt(string("General"), string("LogMaxSize"));

    Module::CLogger::GetInstance().SetLogConf(logLevel, logCount, logMaxSize);
}

static string TrimRightSpace(const string& str)
{
    string strTemp = str;
    for (string::reverse_iterator rIt = strTemp.rbegin(); rIt != strTemp.rend();) {
        int i = 0;
        if (' ' == (*rIt)) {
            string::iterator it = (++rIt).base();
            rIt = string::reverse_iterator(strTemp.erase(it));
        } else {
            ++rIt;
        }
    }

    return strTemp;
}

bool ConfigReaderImpl::AddConfigFile(const string& name)
{
#ifdef WIN32
    std::string fullPath = getConfigPath() + "\\" + name;
    if (name.at(0) == '\\') {
        fullPath = name;
    }
#else
    std::string fullPath = getConfigPath() + "/" + name;
    if (name.at(0) == '/') {
        fullPath = name;
    }
#endif
    HCP_Logger_noid(INFO, MODULE_NAME) << "Add config file. path=" << fullPath << HCPENDLOG;
    m_confFileName.push_back(make_pair(fullPath, 0));
    return true;
}

bool ConfigReaderImpl::isFileModified(pair<string, time_t>& fullFileName)
{
    time_t confLastModifyTime = 0;
    int iRet = CFile::GetlLastModifyTime(fullFileName.first.c_str(), confLastModifyTime);
    if (SUCCESS != iRet) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "Get last modify time failed for " << fullFileName.first << HCPENDLOG;
        return false;
    }

    if (confLastModifyTime != fullFileName.second) {
        fullFileName.second = confLastModifyTime;
        return true;
    }
    return false;
}
/*lint +e830 +e1788*/
string ConfigReaderImpl::getConfigPath() const
{
    return CPath::GetInstance().GetConfPath();
}

bool ConfigReaderImpl::checkConfigInfo(
    const string& sectionName, const string& keyName, const string& value, bool& isInt)
{
    ConfInfoMap::const_iterator ite = m_valueInfo.find(Configkey(sectionName, keyName));
    if (ite == m_valueInfo.end()) {
        return false;
    }

    isInt = false;
    CONFIG_READER_ITEM_VALUE_TYPE type = ite->second->getType();
    if (CONFIG_READER_ITEM_VALUE_INT == type) {
        isInt = true;
        int num = 0;
        if (FAILED == CMpString::StringtoPositiveInt32(value, num)) {
            return false;
        }
        if ((num < ite->second->getMinValue()) || (num > ite->second->getMaxValue())) {
            return false;
        }
    }

    return true;
}

string ConfigReaderImpl::getConfigVersion()
{
    lock_guard<std::mutex> lck(m_versionMtx);

    if (m_versionCache != "") {
        return m_versionCache;
    }

    string versionfile;

    versionfile = CPath::GetInstance().GetConfFilePath("version");
    fstream fs;
    fs.open(versionfile, fstream::in);
    if (!fs.is_open()) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "Open version file error. File is:" << versionfile << HCPENDLOG;
        return m_versionCache;
    }

    getline(fs, m_versionCache);
    fs.close();
    HCP_Logger_noid(INFO, MODULE_NAME) << "Got version from file:" << m_versionCache << HCPENDLOG;
    return m_versionCache;
}

void ConfigReaderImpl::ReadLogLevelFromPM(bool readLoglevelFromPM)
{
    m_readLoglevelFromPM = readLoglevelFromPM;
}

void ConfigReaderImpl::SetUpdateCnf(bool updateCnf)
{
    m_updateCnf = updateCnf;
}

int ConfigReaderImpl::GetLogLevelFromK8s()
{
    return OS_LOG_DEBUG;
}

bool ConfigReaderImpl::GetFileServerSslConfigFromK8s()
{
    return true;
}

void ConfigReaderImpl::FreshFileServerSslConfig()
{
    m_fileServerSslConfig = GetFileServerSslConfigFromK8s();
}

bool ConfigReaderImpl::GetFileServerSslConfig() const
{
    return m_fileServerSslConfig;
}
} // namespace Module