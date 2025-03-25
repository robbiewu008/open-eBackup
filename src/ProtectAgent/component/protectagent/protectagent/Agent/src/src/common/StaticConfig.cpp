/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file StaticConfig.cpp
 * @brief  Contains some config which will not changge when process running.
 * @version 1.1.0
 * @date 2024-09-22
 * @author h00668904
 */
#include <fstream>
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/Utils.h"
#include "common/StaticConfig.h"
#include "common/JsonUtils.h"

namespace StaticConfig {

const std::string BACKUP_NET_PLANE_FILE = "/opt/network-conf/backup_net_plane";

bool IsInnerAgent()
{
    static bool initFlag = false;
    static mp_int32 installType = AGENT_INSTALL_TYPE_EXTERNAL;
    if (initFlag) {
        return (installType == AGENT_INSTALL_TYPE_INTERNAL);
    }

    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_BACKUP_SCENE, installType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get back up scene failed.");
        return false;
    }
    initFlag = true;
    if (installType == AGENT_INSTALL_TYPE_INTERNAL) {
        return true;
    }
    return false;
}

bool NeedClusterEsn()
{
    if (!IsInnerAgent()) {
        return false;
    }
    mp_string deployType;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DEPLOY_TYPE, deployType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get deploy type failed.");
        return false;
    }

    if (deployType == HOST_ENV_DEPLOYTYPE_X8000 ||
        deployType == HOST_ENV_DEPLOYTYPE_X6000 ||
        deployType == HOST_ENV_DEPLOYTYPE_X3000 ||
        deployType == HOST_ENV_DEPLOYTYPE_X9000) {
        return true;
    }
    return false;
}

bool GetAgentIp(mp_string& agentIp)
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_AGENT_IP, agentIp);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get agent ip failed.");
        return false;
    }
    return true;
}

bool IsInnerAgentMainDeploy()
{
    if (!IsInnerAgent()) {
        return false;
    }

    mp_string deployType;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DEPLOY_TYPE, deployType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get deploy type failed.");
        return false;
    }

    if (deployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT ||
        deployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT_NO_BRAND ||
        deployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT_CYBER_ENGINE ||
        deployType == HOST_ENV_DEPLOYTYPE_E6000 ||
        deployType == HOST_ENV_DEPLOYTYPE_DATABACKUP) {
        return false;
    }
    return true;
}

bool GetInnerAgentNodeIps(std::vector<std::string>& ips)
{
    if (!IsInnerAgent()) {
        COMMLOG(OS_LOG_INFO, "This is not inner agent.");
        return false;
    }

    // Get node name
    auto nodeName = getenv("NODE_NAME");
    if (nodeName == nullptr) {
        COMMLOG(OS_LOG_ERROR, "Getenv NODE_NAME failed.");
        return false;
    }

    // Parse backup_net_plane
    std::ifstream infile;
    infile.open(BACKUP_NET_PLANE_FILE.c_str(), std::ifstream::in);
    if ((infile.fail() && infile.bad()) || ((infile.rdstate() & std::ifstream::failbit) != 0)) {
        ERRLOG("Open file %s failed, failed[%d], bad[%d]. errno[%d]:%s.", BACKUP_NET_PLANE_FILE.c_str(),
            infile.fail(), infile.bad(), errno, strerror(errno));
        infile.close();
        return false;
    }

    Json::Reader jsonReader;
    Json::Value netPlaneJson;
    std::string readLine;

    std::string netPlaneString = "";
    while (getline(infile, readLine)) {
        netPlaneString += readLine;
    }
    infile.close();

    if (!jsonReader.parse(netPlaneString, netPlaneJson)) {
        COMMLOG(OS_LOG_ERROR, "parse netPlane failed.");
        return false;
    }

    // Get ips
    for (Json::Value netPlaneIP : netPlaneJson) {
        if (!netPlaneIP.isMember("nodeId") || netPlaneIP["nodeId"] != nodeName) {
            continue;
        }
        if (netPlaneIP.isMember("logic_ip_list")) {
            Json::Value ipListJson = netPlaneIP["logic_ip_list"];
            for (Json::Value ipJson : ipListJson) {
                COMMLOG(OS_LOG_DEBUG, "Get logic ip:%s.", ipJson["ip"].asString().c_str());
                ips.push_back(ipJson["ip"].asString());
            }
        }
    }

    return true;
}
};