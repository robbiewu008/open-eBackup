#ifndef _AGENT_PLUGIN_CFG_PARSE_H
#define _AGENT_PLUGIN_CFG_PARSE_H

#include <vector>
#include "common/Types.h"
#include "common/CMpThread.h"
#include "tinyxml/tinyxml2.h"

static const mp_string XML_NODE_PLUGIN_LIST = "PluginList";
static const mp_string XML_NODE_PLUGIN = "Plugin";
static const mp_string XML_ATTR_PLUGIN_NAME = "name";
static const mp_string XML_ATTR_PLUGIN_VERSION = "version";
static const mp_string XML_ATTR_PLUGIN_SERVICE = "service";
static const mp_string XML_ATTR_PLUGIN_LAZYLOAD = "lazyload";

typedef struct tag_plugin_def_tag {
    mp_string name;
    mp_string version;
    mp_string service;
    mp_bool lazyload;  // 插件默认为惰性加载； lazyload = 1或者没有配置时为惰性加载；
} plugin_def_t;

class PluginCfgParse {
public:
    PluginCfgParse();
    ~PluginCfgParse();

    mp_int32 Init(const mp_string& pszFileName);
    mp_int32 GetPluginByService(const mp_string& strServiceName, plugin_def_t& plgDef);
    mp_int32 GetPluginVersion(const mp_string& pszPlgName, mp_string& strVersion);
    mp_void GetPreLoadPlugins(std::vector<plugin_def_t>& vecPlgDefs);
    mp_void GetPreLoadPlugins(std::vector<mp_string>& vecPlgNames);
    void PrintPluginDef();

private:
    mp_string m_strFileName;
    mp_time m_tFileTime;
    std::vector<plugin_def_t> m_vecPlgDefs;
    thread_lock_t m_tLock;

private:
    mp_bool LoadCfg(const mp_string& pszFileName);
    mp_bool LoadPluginDefs(tinyxml2::XMLElement& pTiPlugins);
    mp_bool LoadPluginDef(tinyxml2::XMLElement& pTiPlugin);
    mp_void AddPluginDef(plugin_def_t& plgDef);
    plugin_def_t* GetPlugin(const mp_string& pszPluginName);
};

#endif  // _AGENT_PLUGIN_CFG_PARSE_H
