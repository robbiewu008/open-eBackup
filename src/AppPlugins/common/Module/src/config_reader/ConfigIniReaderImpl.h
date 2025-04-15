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
#ifndef CONFIGREADERIMPL_H
#define CONFIGREADERIMPL_H

#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <mutex>
#include <atomic>
#include "tinyxml2.h"
#include "common/Thread.h"
#include "common/Utils.h"

namespace Module {

class ConfigValueBase;

struct Configkey
{
    std::string sectionName;
    std::string keyName;

    Configkey(const std::string & inSectionName, const std::string & inKeyName)
        :sectionName(inSectionName),
         keyName(inKeyName)
    {}

    bool operator < (const Configkey & other) const
    {
        if (sectionName != other.sectionName) {
            return sectionName < other.sectionName;
        }

        return keyName < other.keyName;
    }
};

typedef std::map<Configkey, ConfigValueBase*> ConfInfoMap;

#ifdef  WIN32
class AGENT_API ConfigReaderImpl
#else
class ConfigReaderImpl
#endif
{
public:
    static ConfigReaderImpl * instance();
    static void destroy();

    int getInt(const std::string & sectionName, const std::string & keyName, bool logFlag=true) const; 

    std::string getString(const std::string & sectionName, const std::string & keyName, bool logFlag=true) const;

    std::string GetStringFromAgentXml(const std::string& sectionName, const std::string& keyName) const;

    void refresh();

    void refresh(const std::vector<std::string>& confFileName);

    void setLogConf();

    void notifyTimmerEnd();

    void putIntConfigInfo(const std::string & sectionName, const std::string & keyName, int minValue, int maxValue, int defaultValue);

    void putStringConfigInfo(const std::string & sectionName, const std::string & keyName, const std::string & defaultValue);

    std::string GetBackupSceneFromXml(const std::string& strSection) const;
    
    void putIPConfigInfo(const std::string & sectionName, const std::string & keyName, const std::string & defaultValue);

    bool checkConfigInfo(const std::string & , const std::string & , const std::string & , bool & );

    int GetLogLevelFromK8s();
    bool GetFileServerSslConfig() const;
    void ReadLogLevelFromPM(bool readLoglevelFromPM);
    void SetUpdateCnf(bool updateCnf);
    virtual ~ConfigReaderImpl();
    std::string getConfigVersion();
    std::vector<std::string> getConfigFiles();

private:
    static ConfigReaderImpl * m_pInstance;
    static thread_lock_t m_instMutext;
    static thread_lock_t m_configMutext;
    static thread_lock_t m_configFileMutext;
    std::mutex m_versionMtx;

    thread_id_t m_timer;
    ConfInfoMap m_valueInfo;
    bool m_running;
    bool m_readLoglevelFromPM;
    bool m_updateCnf = true;  // 是否开启检查agent配置文件或PM配置文件的变化更新配置文件，GeneralDB的rpctool进程需设置为false
    std::vector<std::pair<std::string, std::time_t> > m_confFileName; //second is the file last modify time
    std::pair<std::string, std::time_t> m_pluginConfFile;
    std::pair<std::string, std::time_t> m_agentConfFile;
    std::map<std::string, std::string> logLevelAdnValue;
    std::string m_versionCache;

    ConfigReaderImpl();

    ConfigReaderImpl(const ConfigReaderImpl & other)
    {
        m_timer = other.m_timer;
        m_valueInfo = other.m_valueInfo;
        m_running = other.m_running;
        m_confFileName = other.m_confFileName;
    };

    ConfigReaderImpl & operator = (const ConfigReaderImpl & other)
    {
        if (this != &other) {
            m_timer = other.m_timer;
            m_valueInfo = other.m_valueInfo;
            m_running = other.m_running;
            m_confFileName = other.m_confFileName;
        }
        return *this;
    };



	void clear();

    void InitConfigInfos();

    void loadConfigFiles();

    bool AddConfigFile(const std::string& name);

    bool isFileModified(std::pair<std::string, std::time_t> & fullFileName);

    std::string GetLogLevelFromXml(std::string& xmlFilePath, const std::string& strSection) const;

    tinyxml2::XMLElement* GetChildElement(tinyxml2::XMLElement* pParentElement, const std::string& strSection) const;

    bool ModifyConfigurationFile(std::string configPathToRead);

    bool ReadPmLogLevel(std::string pluginConfigFile, std::string &logLevel);

    bool IsLogLevelValue(const std::string& logLevel) const;

    bool UpdatePluginConfigFile(std::string logLevel);

#ifdef WIN32
    static DWORD WINAPI TimerThreadProc(void* pConfigReader);
#else
    static void* TimerThreadProc(void* pConfigReader);
#endif

    void ConfigReaderTimer();

    std::string getConfigPath() const;

    void InitConfigInfosForGeneral();
    void InitConfigInfosForGeneralForOther();
    void InitConfigInfosForAdminNode();
    void InitConfigInfosForBackupNode();
    void InitConfigInfosForBackupNodeForCommonTask();
    void InitConfigInfosForBackupNodeForS3();
    void InitConfigInfosForBackupNodeForReadWrite();
    void InitConfigInfosForBackupNodeForBackupAndRestore();
    void InitConfigInfoForDPA();
    void InitConfigInfoForBackupService();
    void InitConfigInfoForJobBrickWorkload();
    void InitConfigInfoForMicroService();
    void InitConfigInfoForDataBase();
    void InitConfigInfoForReplication();
    void InitConfigInfoForArchive();
    void InitArchiveDmmConfigForFileServer();
    void InitConfigInfoForJobManager();
    void InitConfigInfoForWDS();
    void InitConfigInfoForHCS();
    void InitConfigInfoForFilePlugin();

    std::atomic<bool> m_fileServerSslConfig { true };
    bool GetFileServerSslConfigFromK8s();
    void FreshFileServerSslConfig();
    void initConfigInfos111();
};

} // namespace Module

#endif

