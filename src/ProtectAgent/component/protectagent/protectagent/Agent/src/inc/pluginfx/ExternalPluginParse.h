#ifndef _EXTERNAL_PLUGIN_PARSE_H
#define _EXTERNAL_PLUGIN_PARSE_H

#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include "common/Types.h"
#include "common/Defines.h"
#include "common/JsonUtils.h"

struct app_version {
    mp_string minVer;
    mp_string maxVer;
};

struct plugin_info {
    std::vector<mp_string> application; // 插件支持应用
    mp_string appTypeStr;  // 存放配置文件中应用类型字段
    mp_string name;        // 插件名
    mp_string version;     // 插件版本
    std::map<mp_string, app_version> application_version; // 应用版本
    mp_string startUser;   // 启动用户
    mp_int32 cpuLimit;    // 单位1%，用于设置 cpu.cfs_quotas_us 缺省值 -1
    mp_int32 memoryLimit;    // 单位M，用于设置 memory.limit_in_bytes 缺省值 -1
    mp_int32 blkioWeight;    // 用于设置 blkio.weight 缺省值 -1
    std::map<mp_string, mp_string> mountParam; // 应用挂载参数
};

class ExternalPluginParse {
public:
    ExternalPluginParse();
    ~ExternalPluginParse();
    mp_int32 Init();
    mp_int32 GetPluginAppType(const mp_string &strPluginListName, mp_string &pluginAppType);
    mp_int32 GetPluginName(const mp_string &strPluginListName, mp_string &pluginName);
    mp_int32 GetPluginVersion(const mp_string &strPluginListName, mp_string &pluginVersion);
    const std::vector<mp_string>& GetPluginList()
    {
        return m_pluginList;
    }
    mp_string GetStartUser(const mp_string &pluginName);
    mp_int32 GetPluginNameByAppType(const mp_string &appType, mp_string &pluginName);
    mp_int32 GetSubJobCntByAppType(const mp_string &appType, mp_int32 &value);
    std::map<mp_string, plugin_info> GetPluginsInfo()
    {
        return m_pluginsInfo;
    }
    mp_string GetMountParamByAppType(const mp_string &appType, const mp_string &mountOptionKey);
    mp_int32 GetCpuLimitByPluginName(const mp_string &pluginName);
    mp_int32 GetMemoryLimitByPluginName(const mp_string &pluginName);
    mp_int32 GetBlkioWeightByPluginName(const mp_string &pluginName);

private:
    mp_int32 ParsePluginAttribute();
    mp_void GetPluginConfigVersion(const mp_string &strPluginConfigName, mp_string &strVersion);
    mp_int32 GetConfigInfo(plugin_info &pluginInfo, const mp_string &strFileName);
    mp_int32 ParseJsonFileld(plugin_info &pluginInfo, const Json::Value &jsonValue);
    mp_void GetPluginMountParam(const Json::Value &jsonValue, std::map<mp_string, mp_string> &mountParam);
    mp_void GetApplicationSubJobCntMax(const Json::Value &jsonValue, const std::vector<mp_string> &application);

private:
    std::map<mp_string, plugin_info> m_pluginsInfo; // 插件文件信息
    std::vector<mp_string> m_pluginList; // 插件文件名称列表
    std::map<mp_string, mp_int32> m_SubJobCnt;    // 各个应用设置的子任务数量
    std::mutex m_Mutex;
};

#endif // _EXTERNAL_PLUGIN_PARSE_H