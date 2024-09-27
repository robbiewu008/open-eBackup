/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file PluginCfgParse.cpp
 * @brief  The implemention about Agent
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "pluginfx/PluginCfgParse.h"
#include "common/Log.h"
#include "securec.h"
#include "common/Defines.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
using namespace std;
using namespace tinyxml2;

PluginCfgParse::PluginCfgParse()
{
    m_tFileTime = 0;
    CMpThread::InitLock(&m_tLock);
}

PluginCfgParse::~PluginCfgParse()
{
    CMpThread::DestroyLock(&m_tLock);
}

mp_int32 PluginCfgParse::Init(const mp_string& pszFileName)
{
    DBGLOG("Begin init plugin conf parse.");
    if (pszFileName.empty()) {
        ERRLOG("Input param is null.");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    CThreadAutoLock tlock(&m_tLock);
    if (!LoadCfg(pszFileName)) {
        ERRLOG("Load cfg file failed, file %s.", BaseFileName(pszFileName).c_str());
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    DBGLOG("Init plugin conf parse succ.");
    return MP_SUCCESS;
}

mp_int32 PluginCfgParse::GetPluginByService(const mp_string& strServiceName, plugin_def_t& plgDef)
{
    CThreadAutoLock tlock(&m_tLock);

    DBGLOG("#### strServiceName:%s", strServiceName.c_str());
    for (std::vector<plugin_def_t>::iterator iter = m_vecPlgDefs.begin(); iter != m_vecPlgDefs.end(); ++iter) {
        DBGLOG("#### name:%s, server:%s, version:%s", iter->name.c_str(), iter->service.c_str(), iter->version.c_str());
    }

    for (mp_uint32 i = 0; i < m_vecPlgDefs.size(); i++) {
        if (strcmp(strServiceName.c_str(), m_vecPlgDefs[i].service.c_str()) == 0) {
            plgDef = m_vecPlgDefs[i];
            return MP_SUCCESS;
        }
    }

    return MP_FAILED;
}

mp_void PluginCfgParse::GetPreLoadPlugins(vector<plugin_def_t>& vecPlgDefs)
{
    CThreadAutoLock tlock(&m_tLock);

    for (std::vector<plugin_def_t>::iterator iter = m_vecPlgDefs.begin(); iter != m_vecPlgDefs.end(); ++iter) {
        COMMLOG(OS_LOG_DEBUG,
            "#### name:%s, server:%s, version:%s, lazyload %d",
            iter->name.c_str(),
            iter->service.c_str(),
            iter->version.c_str(),
            iter->lazyload);
    }

    for (mp_uint32 i = 0; i < m_vecPlgDefs.size(); i++) {
        if (!m_vecPlgDefs[i].lazyload) {
            vecPlgDefs.push_back(m_vecPlgDefs[i]);
        }
    }
}

mp_void PluginCfgParse::GetPreLoadPlugins(vector<mp_string>& vecPlgNames)
{
    CThreadAutoLock tlock(&m_tLock);

    for (std::vector<plugin_def_t>::iterator iter = m_vecPlgDefs.begin(); iter != m_vecPlgDefs.end(); ++iter) {
        COMMLOG(OS_LOG_DEBUG,
            "#### name:%s, server:%s, version:%s, lazyload %d",
            iter->name.c_str(),
            iter->service.c_str(),
            iter->version.c_str(),
            iter->lazyload);
    }

    for (mp_uint32 i = 0; i < m_vecPlgDefs.size(); i++) {
        if (!m_vecPlgDefs[i].lazyload) {
            vecPlgNames.push_back(m_vecPlgDefs[i].name);
        }
    }
}

mp_int32 PluginCfgParse::GetPluginVersion(const mp_string& pszPlgName, mp_string& strVersion)
{
    plugin_def_t* pPlgDef;

    COMMLOG(OS_LOG_DEBUG, "Begin get plugin version.");
    if (pszPlgName.empty()) {
        COMMLOG(OS_LOG_DEBUG, "Input param is null.");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    CThreadAutoLock tlock(&m_tLock);
    pPlgDef = GetPlugin(pszPlgName);
    if (pPlgDef == NULL) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    strVersion = pPlgDef->version;
    COMMLOG(OS_LOG_DEBUG, "Get plugin version succ, name %s, version %s.", pszPlgName.c_str(), strVersion.c_str());
    return MP_SUCCESS;
}

void PluginCfgParse::PrintPluginDef()
{
    vector<plugin_def_t>::iterator iter;

    CThreadAutoLock tlock(&m_tLock);
    for (iter = m_vecPlgDefs.begin(); iter != m_vecPlgDefs.end(); ++iter) {
        COMMLOG(OS_LOG_DEBUG, "------Plugin : name %s, version %s-------", iter->name.c_str(), iter->version.c_str());
    }

    COMMLOG(OS_LOG_DEBUG, "");
}

mp_bool PluginCfgParse::LoadCfg(const mp_string& pszFileName)
{
    tinyxml2::XMLDocument xdoc;
    XMLElement* pPlgList = NULL;

    COMMLOG(OS_LOG_DEBUG, "Begin load cfg file.");

    if (pszFileName.empty()) {
        COMMLOG(OS_LOG_ERROR, "Input param is null.");
        return MP_FALSE;
    }
    // CodeDexÎó±¨£¬Dead Code
    if (MP_SUCCESS != CMpFile::GetlLastModifyTime(pszFileName.c_str(), m_tFileTime)) {
        COMMLOG(OS_LOG_ERROR, "Get last modify time failed, file %s.", BaseFileName(pszFileName).c_str());
        return MP_FALSE;
    }
    // CodeDexÎó±¨,KLOCWORK.NPD.FUNC.MUST
    if (xdoc.LoadFile(pszFileName.c_str()) || xdoc.RootElement() == NULL) {
        COMMLOG(OS_LOG_ERROR,
            "Invalid config file '%s', error: %s, %s.",
            BaseFileName(pszFileName).c_str(),
            xdoc.Value(),
            xdoc.ErrorID());
        return MP_FALSE;
    }

    pPlgList = xdoc.RootElement()->FirstChildElement();
    if (pPlgList == NULL) {
        COMMLOG(OS_LOG_INFO, "No 'ModuleList' in config file '%s'.", BaseFileName(m_strFileName).c_str());
    } else {
        COMMLOG(OS_LOG_DEBUG, "start load <%s><%s>", XML_NODE_PLUGIN_LIST.c_str(), XML_NODE_PLUGIN.c_str());

        if (MP_FALSE == LoadPluginDefs(*pPlgList)) {
            COMMLOG(OS_LOG_ERROR, "Load plugins failed.");
            return MP_FALSE;
        }
    }

    COMMLOG(OS_LOG_DEBUG, "Load cfg file succ.");
    return MP_TRUE;
}

mp_bool PluginCfgParse::LoadPluginDefs(XMLElement& pTiPlugins)
{
    XMLElement* plugin = NULL;
    COMMLOG(OS_LOG_DEBUG, "Begin load plugin defs.");

    for (plugin = pTiPlugins.FirstChildElement(); plugin; plugin = plugin->NextSiblingElement()) {
        if (MP_FALSE == LoadPluginDef(*plugin)) {
            COMMLOG(OS_LOG_ERROR, "Load plugin failed.");
            return MP_FALSE;
        }
    }

    COMMLOG(OS_LOG_DEBUG, "Load plugin defs succ.");
    return MP_TRUE;
}

mp_bool PluginCfgParse::LoadPluginDef(XMLElement& pTiPlugin)
{
    plugin_def_t plgDef;
    COMMLOG(OS_LOG_DEBUG, "Begin load plugin def.");

    const mp_char* pszName = pTiPlugin.Attribute(XML_ATTR_PLUGIN_NAME.c_str());
    const mp_char* pszVersion = pTiPlugin.Attribute(XML_ATTR_PLUGIN_VERSION.c_str());
    const mp_char* pszService = pTiPlugin.Attribute(XML_ATTR_PLUGIN_SERVICE.c_str());
    const mp_char* pszLazyload = pTiPlugin.Attribute(XML_ATTR_PLUGIN_LAZYLOAD.c_str());

    if (strempty(pszName) || strempty(pszVersion) || strempty(pszService) || strempty(pszLazyload)) {
        COMMLOG(OS_LOG_ERROR, "No attribute 'name', 'version', 'service' or 'lazyload' in plugin's config.");
        return MP_FALSE;
    }

    plgDef.name = pszName;
    plgDef.version = pszVersion;
    plgDef.service = pszService;
    mp_int32 iValue = atoi(pszLazyload);
    if (iValue != 0) {
        plgDef.lazyload = MP_TRUE;
    } else {
        plgDef.lazyload = MP_FALSE;
    }
    AddPluginDef(plgDef);

    COMMLOG(OS_LOG_DEBUG, "Load plugin def succ.");
    return MP_TRUE;
}

mp_void PluginCfgParse::AddPluginDef(plugin_def_t& plgDef)
{
    if (NULL == GetPlugin(plgDef.name)) {
        m_vecPlgDefs.push_back(plgDef);
    }
}

plugin_def_t* PluginCfgParse::GetPlugin(const mp_string& pszPluginName)
{
    plugin_def_t* pPlgDef = NULL;

    for (mp_uint32 i = 0; i < m_vecPlgDefs.size(); i++) {
        if (pszPluginName == m_vecPlgDefs[i].name) {
            pPlgDef = &m_vecPlgDefs[i];
        }
    }

    return pPlgDef;
}
