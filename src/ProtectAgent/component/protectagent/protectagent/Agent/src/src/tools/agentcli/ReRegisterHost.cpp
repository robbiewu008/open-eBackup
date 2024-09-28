#include "tools/agentcli/ReRegisterHost.h"

#include <iostream>
#include "common/Log.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/Ip.h"
#include "common/MpString.h"
#include "common/CSystemExec.h"
#include "message/tcp/CSocket.h"
#include "tools/agentcli/RegisterHost.h"

mp_int32 ReRegisterHost::Handle()
{
    CHECK_FAIL_EX(InitParam());

    std::cout << "Obtaining ProtectAgent network adapter information. Please wait..." << std::endl;
    INFOLOG("Obtaining ProtectAgent network adapter information. Please wait...");

    if (GenerateAgentIP(false) != MP_SUCCESS) {
        std::cout << "Business ip is not valid, use manage ip." << std::endl;
        WARNLOG("Business ip is not valid, use manage ip.");
        if (GenerateAgentIP(true) != MP_SUCCESS) {
            std::cout << "All ProtectManager IP addresses cannot be accessed." << std::endl;
            ERRLOG("All ProtectManager IP addresses cannot be accessed.");
            return MP_FAILED;
        }
    }
    std::cout << "ProtectManager IP addresses (" << (m_bUseManager ? m_pmManagerIpList : m_pmIpList)
        << ") can be accessed." << std::endl;
    INFOLOG("ProtectManager IP addresses (%s) can be accessed.",
        m_bUseManager ? m_pmManagerIpList.c_str() : m_pmIpList.c_str());
    std::cout << "The Nginx listening IP address is (" << m_nginxIp << ")." << std::endl;
    INFOLOG("The Nginx listening IP address is (%s).", m_nginxIp.c_str());

    CHECK_FAIL_EX(GetChoice());

    if (CSystemExec::ExecSystemWithoutEcho(CPath::GetInstance().GetBinFilePath(STOP_SCRIPT)) == MP_SUCCESS) {
        std::cout << "The DataBackup ProtectAgent service has been successfully stopped." << std::endl;
        INFOLOG("The DataBackup ProtectAgent service has been successfully stopped.");
    } else {
        std::cout << "The DataBackup ProtectAgent service fails to be stopped." << std::endl;
        ERRLOG("The DataBackup ProtectAgent service fails to be stopped.");
    }

    CHECK_FAIL_EX(SetNewNetParam());
    
    if (RegisterHost::Handle("RegisterHost", "", "") != MP_SUCCESS) {
        std::cout << "Re-register host to ProtectManager failed, will rollback net param." << std::endl;
        ERRLOG("Re-register host to ProtectManager failed, will rollback net param.");
        RollbackNetParam();
    } else {
        std::cout << "Re-register host to ProtectManager successfully." << std::endl;
        INFOLOG("Re-register host to ProtectManager successfully.");
    }

    if (CSystemExec::ExecSystemWithoutEcho(CPath::GetInstance().GetBinFilePath(START_SCRIPT)) == MP_SUCCESS) {
        std::cout << "The DataBackup ProtectAgent service has been successfully started." << std::endl;
        INFOLOG("The DataBackup ProtectAgent service has been successfully started.");
        return MP_SUCCESS;
    } else {
        std::cout << "The DataBackup ProtectAgent service fails to be started." << std::endl;
        ERRLOG("The DataBackup ProtectAgent service fails to be started.");
        return MP_FAILED;
    }
}

mp_int32 ReRegisterHost::InitParam()
{
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    std::vector<mp_string> vecOutput;
    CHECK_FAIL_EX(CMpFile::ReadFile(strFilePath, vecOutput));
    for (const mp_string& it : vecOutput) {
        if (it.find("PM_IP=") == 0) {
            m_pmIpList = it.substr(it.find("=", 0) + 1);
        } else if (it.find("PM_PORT=") == 0) {
            m_pmPort = atoi(it.substr(it.find("=", 0) + 1).c_str());
        } else if (it.find("PM_MANAGER_IP=") == 0) {
            m_pmManagerIpList = it.substr(it.find("=", 0) + 1);
        } else if (it.find("PM_MANAGER_PORT=") == 0) {
            m_pmManagerPort = atoi(it.substr(it.find("=", 0) + 1).c_str());
        }
    }

    CHECK_FAIL_EX(CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, m_oldPmIpList));
    CHECK_FAIL_EX(CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_IAM_PORT, m_oldPmPort));

    if (!GetNginxListenIP(m_oldNginxIp, m_oldNginxPort)) {
        ERRLOG("GetNginxListenIP failed.");
        return MP_FAILED;
    }
    
    CHECK_FAIL_EX(CIP::GetHostIPList(m_srcIpv4List, m_srcIpv6List));
    return MP_SUCCESS;
}

mp_int32 ReRegisterHost::GenerateAgentIP(bool bUseManager)
{
    std::vector<mp_string> dstIpList;
    CMpString::StrSplitEx(dstIpList, bUseManager ? m_pmManagerIpList : m_pmIpList, ",");
    if (dstIpList.empty()) {
        ERRLOG("ProtectManager IP (%s) is invalid.", bUseManager ? m_pmManagerIpList.c_str() : m_pmIpList.c_str());
        return MP_FAILED;
    }

    std::vector<mp_string> srcIpList = CIP::IsIPV4(dstIpList.front()) ? m_srcIpv4List : m_srcIpv6List;
    for (const mp_string& strSrcIp : srcIpList) {
        for (const mp_string& strDstIp : dstIpList) {
            if (CSocket::CheckHostLinkStatus(
                strSrcIp, strDstIp, bUseManager ? m_pmManagerPort : m_pmPort) == MP_SUCCESS) {
                m_nginxIp = strSrcIp;
                m_bUseManager = bUseManager;
                return MP_SUCCESS;
            }
        }
    }
    return MP_FAILED;
}

mp_int32 ReRegisterHost::GetChoice()
{
    std::cout << "This script will stop the agent and re-register the agent."
        "Are you sure that no task is running?(y|n)" << std::endl;
    std::cout << "Your choice:";
    mp_string userIn;
    std::cin >> userIn;
    if (userIn.empty()) {
        std::cout << "Default selection y." << std::endl;
        return MP_SUCCESS;
    } else if (userIn == "n" || userIn == "no") {
        return MP_FAILED;
    } else if (userIn == "y" || userIn == "yes") {
        return MP_SUCCESS;
    } else {
        std::cout << "Please enter y or n." << std::endl;
        return MP_FAILED;
    }
}

mp_int32 ReRegisterHost::SetNewNetParam()
{
    CHECK_FAIL_EX(CConfigXmlParser::GetInstance().SetValue(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP,
        m_bUseManager ? m_pmManagerIpList : m_pmIpList));
    CHECK_FAIL_EX(CConfigXmlParser::GetInstance().SetValue(CFG_BACKUP_SECTION, CFG_IAM_PORT,
        std::to_string(m_bUseManager ? m_pmManagerPort : m_pmPort)));

    std::string strNginxConfig = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    std::vector<mp_string> vecOutput;
    CHECK_FAIL_EX(CMpFile::ReadFile(strNginxConfig, vecOutput));
    for (mp_string& it : vecOutput) {
        if (it.find("listen") != mp_string::npos) {
            it = "        listen       " + m_nginxIp + ":" + std::to_string(m_oldNginxPort) + " ssl;";
            break;
        }
    }
    CHECK_FAIL_EX(CIPCFile::WriteFile(strNginxConfig, vecOutput));
    return MP_SUCCESS;
}

mp_void ReRegisterHost::RollbackNetParam()
{
    CConfigXmlParser::GetInstance().SetValue(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, m_oldPmIpList);
    CConfigXmlParser::GetInstance().SetValue(CFG_BACKUP_SECTION, CFG_IAM_PORT, std::to_string(m_oldPmPort));

    std::string strNginxConfig = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    std::vector<mp_string> vecOutput;
    CMpFile::ReadFile(strNginxConfig, vecOutput);
    for (mp_string& it : vecOutput) {
        if (it.find("listen") != mp_string::npos) {
            it = "        listen       " + m_oldNginxIp + ":" + std::to_string(m_oldNginxPort) + " ssl;";
            break;
        }
    }
    CIPCFile::WriteFile(strNginxConfig, vecOutput);
}
