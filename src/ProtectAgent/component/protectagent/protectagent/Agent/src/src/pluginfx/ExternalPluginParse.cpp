#include "pluginfx/ExternalPluginParse.h"
#include <string>
#include <fstream>
#include <algorithm>
#include "common/Types.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/File.h"

namespace {
    const mp_string PLUGIN_APPLICATION = "application";
    const mp_string PLUGIN_NAME = "name";
    const mp_string PLUGIN_APP_VERSION = "application_version"; // 插件应用版本
    const mp_string PLUGIN_MIN_VERSION = "min_version";
    const mp_string PLUGIN_MAX_VERSION = "max_version";
    const mp_string PLUGIN_MOUNT = "mount"; // 插件挂载参数
    const mp_string PLUGIN_CONFIG_NAME = "plugin_attribute_"; // 插件特性配置文件名：plugin_attribute_{version}.json
    const mp_string JSON_FORMAT_NAME = ".json";
    const mp_string PLUGIN_START_USER = "run_account";
    const mp_string PLUGIN_CPU_LIMIT = "cpu_limit";
    const mp_string PLUGIN_MEMORY_LIMIT = "memory_limit";
    const mp_string PLUGIN_BLKIO_WEIGHT = "blkio_weight";
    const mp_int32 CGROUP_LIMIT_UNSET = -1;  // cgroup默认值
    const mp_string PLUGIN_APP_SUB_JOB_CNT_MAX = "application_sub_job_cnt_max"; // 插件应用允许的最大子任务数量
    const mp_int32 SUBJOB_CNT_MIN_RANGE = 1;
    const mp_int32 SUBJOB_CNT_MAX_RANGE = 100000;
};

ExternalPluginParse::ExternalPluginParse()
{
    Init();
}

ExternalPluginParse::~ExternalPluginParse()
{}

mp_int32 ExternalPluginParse::Init()
{
    DBGLOG("Start to parse plugin attribute.");
    return ParsePluginAttribute();
}

mp_int32 ExternalPluginParse::GetPluginAppType(const mp_string &strPluginListName, mp_string &pluginAppType)
{
    std::map<mp_string, plugin_info>::iterator iter = m_pluginsInfo.find(strPluginListName);
    if (iter != m_pluginsInfo.end()) {
        pluginAppType = m_pluginsInfo[strPluginListName].appTypeStr;
        return MP_SUCCESS;
    }

    return MP_FAILED;
}

mp_int32 ExternalPluginParse::GetPluginName(const mp_string &strPluginListName, mp_string &pluginName)
{
    std::map<mp_string, plugin_info>::iterator iter = m_pluginsInfo.find(strPluginListName);
    if (iter != m_pluginsInfo.end()) {
        pluginName = m_pluginsInfo[strPluginListName].name;
        return MP_SUCCESS;
    }

    return MP_FAILED;
}

mp_int32 ExternalPluginParse::GetPluginVersion(const mp_string &strPluginListName, mp_string &pluginVersion)
{
    std::map<mp_string, plugin_info>::iterator iter = m_pluginsInfo.find(strPluginListName);
    if (iter != m_pluginsInfo.end()) {
        pluginVersion = m_pluginsInfo[strPluginListName].version;
        return MP_SUCCESS;
    }

    return MP_FAILED;
}

mp_int32 ExternalPluginParse::ParsePluginAttribute()
{
    mp_string externalPluginDir = CPath::GetInstance().GetRootPath() +
        PATH_SEPARATOR + ".." + PATH_SEPARATOR + EXTERNAL_PLUGIN_PATH;
    // 解析插件目录
    m_pluginList.clear();
    CMpFile::GetFolderDir(externalPluginDir, m_pluginList);
    // 解析所有插件配置文件
    for (int i = 0; i < m_pluginList.size(); i++) {
        if (m_pluginList[i] == "Block_Service") {    // 引入了block_service工具，查询插件时需要过滤掉
            DBGLOG("Delete Block_Service.");
            continue;
        }
        mp_string pluginConfigSuperDir = externalPluginDir + PATH_SEPARATOR + m_pluginList[i];
        std::vector<mp_string> vecName;
        CMpFile::GetFolderFile(pluginConfigSuperDir, vecName);
        mp_string pluginConfigName;
        for (mp_string strName : vecName) {
            if ((strName.find(JSON_FORMAT_NAME.c_str()) != std::string::npos)
                && ((strName.find(PLUGIN_CONFIG_NAME.c_str()) != std::string::npos))) {
                pluginConfigName = strName;
                break;
            }
        }
        if (pluginConfigName.empty()) {
            continue;
        }

        // 获取插件特性配置文件信息
        plugin_info pluginInfo;
        if (GetConfigInfo(pluginInfo, pluginConfigSuperDir + PATH_SEPARATOR + pluginConfigName) != MP_SUCCESS) {
            ERRLOG("Failed to obtain configuration file information.");
            return MP_FAILED;
        }

        // 解析插件版本
        (mp_void)GetPluginConfigVersion(pluginConfigName, pluginInfo.version);
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_pluginsInfo.insert(std::pair<mp_string, plugin_info>(m_pluginList[i], pluginInfo));

        INFOLOG("Plugins is parsed successfully, application[%s], name[%s], version[%s].",
            pluginInfo.appTypeStr.c_str(), pluginInfo.name.c_str(), pluginInfo.version.c_str());
    }
    INFOLOG("Parse plugin attribute finish.");
    return MP_SUCCESS;
}

mp_int32 ExternalPluginParse::GetConfigInfo(plugin_info &pluginInfo, const mp_string &strFileName)
{
    std::ifstream infile;
    infile.open(strFileName.c_str(), std::ifstream::in);
    if ((infile.fail() && infile.bad()) || ((infile.rdstate() & std::ifstream::failbit) != 0)) {
        ERRLOG("Open file %s failed, failed[%d], bad[%d]. errno[%d]:%s.", strFileName.c_str(),
            infile.fail(), infile.bad(), errno, strerror(errno));
        infile.close();
        return MP_FAILED;
    }

    Json::Reader jsonReader;
    Json::Value jsonValue;
    if (!jsonReader.parse(infile, jsonValue)) {
        ERRLOG("strFileName[%s] JsonData is invalid.", strFileName.c_str());
        infile.close();
        return MP_FAILED;
    }

    if (!jsonValue.isObject() || !jsonValue.isMember(PLUGIN_APPLICATION) || !jsonValue.isMember(PLUGIN_NAME) ||
        !jsonValue.isMember(PLUGIN_START_USER)) {
        ERRLOG("The plugin configuration item is incomplete.");
        infile.close();
        return MP_FAILED;
    }

    mp_int32 iRet = ParseJsonFileld(pluginInfo, jsonValue);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to parse the JSON configuration.");
        return MP_FAILED;
    }

    // application 存在多个值，逗号分开
    CMpString::StrSplit(pluginInfo.application, pluginInfo.appTypeStr, ',');
    if (pluginInfo.application.empty()) {
        ERRLOG("Split application field[%s] failed.", pluginInfo.appTypeStr.c_str());
        infile.close();
        return MP_FAILED;
    }
    // 获取插件挂载参数
    GetPluginMountParam(jsonValue, pluginInfo.mountParam);

    // 解析应用子任务数
    GetApplicationSubJobCntMax(jsonValue, pluginInfo.application);
    infile.close();

    return MP_SUCCESS;
}

mp_void ExternalPluginParse::GetApplicationSubJobCntMax(const Json::Value &jsonValue,
    const std::vector<mp_string> &application)
{
    if (!jsonValue.isObject() || !jsonValue.isMember(PLUGIN_APP_SUB_JOB_CNT_MAX.c_str()) ||
        !jsonValue[PLUGIN_APP_SUB_JOB_CNT_MAX].isObject()) {
        WARNLOG("Get field %s from config file failed!", PLUGIN_APP_SUB_JOB_CNT_MAX.c_str());
        return;
    }
    Json::Value subJobCntMaxJsonValue = jsonValue[PLUGIN_APP_SUB_JOB_CNT_MAX];
    // 根据应用读取
    for (auto app : application) {
        mp_int32 cntMax = 0;
        if (CJsonUtils::GetJsonInt32(subJobCntMaxJsonValue, app, cntMax) != MP_SUCCESS) {
            WARNLOG("Get field %s from config file failed!", app.c_str());
            continue;
        }
        if (cntMax < SUBJOB_CNT_MIN_RANGE || cntMax > SUBJOB_CNT_MAX_RANGE) {
            WARNLOG("Read field (%s = %d) from config file is invaild!", app.c_str(), cntMax);
            continue;
        }
        m_SubJobCnt[app] = cntMax;
        DBGLOG("The appOfSubJobCntMax (%s => %d)", app.c_str(), m_SubJobCnt[app]);
    }
}

mp_int32 ExternalPluginParse::GetSubJobCntByAppType(const mp_string &appType, mp_int32 &value)
{
    if (m_SubJobCnt.count(appType) <= 0) {
        return MP_FAILED;
    }
    value = m_SubJobCnt[appType];
    return MP_SUCCESS;
}

mp_void ExternalPluginParse::GetPluginConfigVersion(const mp_string &strPluginConfigName, mp_string &strVersion)
{
    // 插件名：plugin_attribute_{version}.json
    mp_string::size_type start = PLUGIN_CONFIG_NAME.size();
    mp_string::size_type end = strPluginConfigName.find(".json");
    strVersion = strPluginConfigName.substr(start, end - start);
}

mp_void ExternalPluginParse::GetPluginMountParam(const Json::Value &jsonValue,
    std::map<mp_string, mp_string> &mountParam)
{
    if (!jsonValue.isObject() || !jsonValue.isMember(PLUGIN_MOUNT)) {
        return;
    }
    Json::Value::Members members = jsonValue[PLUGIN_MOUNT].getMemberNames();
    for (auto &iter : members) {
        if (jsonValue[PLUGIN_MOUNT][iter].isString()) {
            mountParam.insert(std::pair<mp_string, mp_string>(iter, jsonValue[PLUGIN_MOUNT][iter].asString()));
        }
    }
}

mp_string ExternalPluginParse::GetStartUser(const mp_string &pluginName)
{
    if (m_pluginsInfo.find(pluginName) != m_pluginsInfo.end()) {
        return m_pluginsInfo[pluginName].startUser;
    }
    ERRLOG("Fail to get start user for %s.", pluginName.c_str());
    return "";
}

mp_int32 ExternalPluginParse::GetCpuLimitByPluginName(const mp_string &pluginName)
{
    if (m_pluginsInfo.find(pluginName) != m_pluginsInfo.end()) {
        mp_int32 cpuLimit = m_pluginsInfo[pluginName].cpuLimit;
        DBGLOG("The cpuLimit of %s is %d%%.", pluginName.c_str(), cpuLimit);
        return cpuLimit;
    }
    WARNLOG("Fail to get cpuLimit for %s.", pluginName.c_str());
    return CGROUP_LIMIT_UNSET;
}

mp_int32 ExternalPluginParse::GetMemoryLimitByPluginName(const mp_string &pluginName)
{
    if (m_pluginsInfo.find(pluginName) != m_pluginsInfo.end()) {
        mp_int32 memoryLimit = m_pluginsInfo[pluginName].memoryLimit;
        DBGLOG("The memoryLimit of %s is %dM.", pluginName.c_str(), memoryLimit);
        return memoryLimit;
    }
    WARNLOG("Fail to get memoryLimit for %s.", pluginName.c_str());
    return CGROUP_LIMIT_UNSET;
}

mp_int32 ExternalPluginParse::GetBlkioWeightByPluginName(const mp_string &pluginName)
{
    if (m_pluginsInfo.find(pluginName) != m_pluginsInfo.end()) {
        mp_int32 blkioWeight = m_pluginsInfo[pluginName].blkioWeight;
        DBGLOG("The blkio weight of %s is %d.", pluginName.c_str(), blkioWeight);
        return blkioWeight;
    }
    WARNLOG("Fail to get blkio weight for %s.", pluginName.c_str());
    return CGROUP_LIMIT_UNSET;
}

mp_int32 ExternalPluginParse::GetPluginNameByAppType(const mp_string &appType, mp_string &pluginName)
{
    mp_string targetAppType = appType;
    transform(appType.begin(), appType.end(), targetAppType.begin(), tolower);
    std::map<mp_string, plugin_info>::iterator iter = m_pluginsInfo.begin();
    for (; iter != m_pluginsInfo.end(); ++iter) {
        for (size_t i = 0; i < iter->second.application.size(); ++i) {
            mp_string srcAppType = iter->second.application[i];
            transform(srcAppType.begin(), srcAppType.end(), srcAppType.begin(), tolower);
            if (targetAppType == srcAppType) {
                pluginName = iter->first;
                DBGLOG("Get plugin name(%s) for apptype %s.", pluginName.c_str(), appType.c_str());
                return MP_SUCCESS;
            }
        }
    }
    ERRLOG("Fail to get plugin name for apptype %s.", appType.c_str());
    return MP_FAILED;
}

mp_string ExternalPluginParse::GetMountParamByAppType(const mp_string &appType, const mp_string &mountOptionKey)
{
    mp_string pluginName;
    mp_string mountParam = "";
    if (GetPluginNameByAppType(appType, pluginName) != MP_SUCCESS) {
        return mountParam;
    }
    auto iter = m_pluginsInfo.find(pluginName);
    if (iter != m_pluginsInfo.end()) {
        auto iterMount = iter->second.mountParam.find(mountOptionKey);
        if (iterMount != iter->second.mountParam.end()) {
            mountParam = iterMount->second;
        }
    }
    return mountParam;
}

mp_int32 ExternalPluginParse::ParseJsonFileld(plugin_info &pluginInfo, const Json::Value &jsonValue)
{
    std::vector<Json::Value> vecJsonValue;
    mp_int32 iRet = CJsonUtils::GetJsonArrayJson(jsonValue, PLUGIN_APP_VERSION, vecJsonValue);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetJsonArray application_version failed, iRet = %d.", iRet);
        return iRet;
    }

    for (int i = 0; i < vecJsonValue.size(); i++) {
        mp_string minVer;
        mp_string maxVer;
        mp_string appType;
        app_version applicationVer;
        GET_JSON_STRING(vecJsonValue[i], PLUGIN_MIN_VERSION, minVer);
        GET_JSON_STRING(vecJsonValue[i], PLUGIN_MAX_VERSION, maxVer);
        GET_JSON_STRING(vecJsonValue[i], PLUGIN_APPLICATION, appType);
        applicationVer.minVer = minVer;
        applicationVer.maxVer = maxVer;
        pluginInfo.application_version[appType] = applicationVer;
        COMMLOG(OS_LOG_INFO, "plugin appType[%s], minVer[%s], maxVer[%s]",
            appType.c_str(), applicationVer.minVer.c_str(), applicationVer.maxVer.c_str());
    }

    GET_JSON_STRING(jsonValue, PLUGIN_NAME, pluginInfo.name);
    GET_JSON_STRING(jsonValue, PLUGIN_APPLICATION, pluginInfo.appTypeStr);
    GET_JSON_STRING(jsonValue, PLUGIN_START_USER, pluginInfo.startUser);
    
    pluginInfo.cpuLimit = CGROUP_LIMIT_UNSET;
    CJsonUtils::GetJsonInt32(jsonValue, PLUGIN_CPU_LIMIT, pluginInfo.cpuLimit);
    pluginInfo.memoryLimit = CGROUP_LIMIT_UNSET;
    CJsonUtils::GetJsonInt32(jsonValue, PLUGIN_MEMORY_LIMIT, pluginInfo.memoryLimit);
    pluginInfo.blkioWeight = CGROUP_LIMIT_UNSET;
    CJsonUtils::GetJsonInt32(jsonValue, PLUGIN_BLKIO_WEIGHT, pluginInfo.blkioWeight);
    return MP_SUCCESS;
}
