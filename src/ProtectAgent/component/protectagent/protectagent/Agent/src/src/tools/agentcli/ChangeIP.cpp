#include "tools/agentcli/ChangeIP.h"
#include "tools/agentcli/RegisterHost.h"
#include "common/Defines.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Ip.h"
#include "host/host.h"
#include "common/CSystemExec.h"
#include "securecom/Password.h"

#ifdef LINUX
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#endif
using namespace std;
/* ------------------------------------------------------------
Description  : 修改Ngnix绑定IP地址
Return       : MP_SUCCESS -- 成功
------------------------------------------------------------- */
mp_int32 ChangeIP::Handle()
{
#ifndef WIN32
    if (getuid() == 0) {
        printf("%s", "Root can not change ip, please su to rdadmin.\n");
        return MP_FAILED;
    }
#endif
    mp_string strInput;
    mp_uint32 iInputFailedTimes = 0;
    while (iInputFailedTimes <= MAX_FAILED_COUNT) {
        printf("%s", INPUT_IP_HINT);
        CPassword::GetInput(INPUT_IP_HINT, strInput);
        // 判断ip地址合法性
        if (!CIP::CheckIsIPv6OrIPv4(strInput) && strInput != ANY_IP) {
            printf("%s\n", INVALID_IP_ADDRESS);
            iInputFailedTimes++;
            continue;
        }
        // 判断是否是本地ip地址
        if (!IsLocalIP(strInput)) {
            printf("%s\n", NOT_LOCAL_IP_ADDRESS);
            iInputFailedTimes++;
            continue;
        }
        // 判断ip地址是否是当前ip地址
        mp_string strCurrentIP;
        mp_int32 iRet = GetIPAddress(strCurrentIP);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "GetIPAddress failed, iRet = %d.", iRet);
            printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
            return iRet;
        }
        if (strcmp(strCurrentIP.c_str(), strInput.c_str()) == 0) {
            printf("%s\n", SAME_IP_ADDRESS);
            iInputFailedTimes++;
            continue;
        }

        ChangeIPAfter(strInput);

        iRet = RegisterHost::GetPMIPandPort();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get PM ip and port failed, iRet = %d.", iRet);
            return iRet;
        }
        iRet = RegisterHost::RegisterHost2PM();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Register to PM failed, iRet = %d.", iRet);
            return iRet;
        }
        return MP_SUCCESS;
    }

    printf("Input wrong IP address over 3 times.\n");
    return MP_FAILED;
}

mp_int32 ChangeIP::ChangeIPAfter(mp_string& strInput)
{
    mp_int32 iRet = SetIPAddress(strInput);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "SetIPAddress failed, iRet = %d.", iRet);
        printf("%s\n", SAME_IP_ADDRESS);
        return iRet;
    } else {
        printf("%s", CHANGE_IP_SUCCESS);
        printf("%s ", RESTART_WEB_SERVICE_HINT);
        printf("%s", CONTINUE);
        CPassword::GetInput(CONTINUE, strInput);
        mp_bool bRet2 = (strInput == "y" || strInput == "Y");
        if (bRet2) {
            // 重启nginx
            iRet = RestartNginx();
            if (iRet != MP_SUCCESS) {
                printf("Restart nginx failed.\n");
                return iRet;
            }
            printf("Restart nginx successfully.\n");
            COMMLOG(OS_LOG_INFO, "Change IP address binded by web service successfully.");
        } else {
            printf("Ip will take effect after nginx restart.\n");
            COMMLOG(OS_LOG_INFO, "Change IP address successfully, but not binded Agent.");
        }
        return MP_SUCCESS;
    }
}

/* ------------------------------------------------------------
Description  : 从配置文件中读取当前ip地址
Return       : MP_SUCCESS -- 成功
------------------------------------------------------------- */
mp_int32 ChangeIP::GetIPAddress(mp_string& strIP)
{
    mp_string strPort;
    return CIP::GetListenIPAndPort(strIP, strPort);
}

/* ------------------------------------------------------------
Description  : 将ip地址在配置文件对应位置进行修改
Return       : MP_SUCCESS -- 成功
------------------------------------------------------------- */
mp_int32 ChangeIP::SetIPAddress(const mp_string& strIP)
{
    mp_string strNginxConfFile = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    if (!CMpFile::FileExist(strNginxConfFile)) {
        printf("Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE.c_str());
        COMMLOG(OS_LOG_ERROR, "Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE.c_str());
        return MP_FAILED;
    }

    vector<mp_string> vecResult;
    mp_int32 iRet = CMpFile::ReadFile(strNginxConfFile, vecResult);
    mp_bool bFalg = iRet != MP_SUCCESS || vecResult.size() == 0;
    if (bFalg) {
        COMMLOG(
            OS_LOG_ERROR, "Read nginx config file failed, iRet = %d, size of vecResult is %d.", iRet, vecResult.size());
        return MP_FAILED;
    }

    for (mp_uint32 i = 0; i < vecResult.size(); i++) {
        mp_string strTmp = vecResult[i];
        mp_string::size_type pos = strTmp.find(BIND_IP_TAG, 0);
        if (pos == mp_string::npos) {
            continue;
        }
        mp_string strLast;
        // 先找:
        mp_string::size_type iColPos = strTmp.find_last_of(CHAR_COLON);
        if (iColPos != mp_string::npos) {
            strLast = strTmp.substr(iColPos + 1);
        } else {
            // 再找数字
            std::size_t iNumPos = strTmp.find_first_of(NUM_STRING);
            if (iNumPos == mp_string::npos || iNumPos <= strlen(BIND_IP_TAG.c_str())) {
                COMMLOG(OS_LOG_ERROR, "Nginx config file is corrupt, strTmp = %s.", strTmp.c_str());
                return MP_FAILED;
            }
            strLast = strTmp.substr(iNumPos);
        }
        vecResult[i] = mp_string(EIGHT_BALNK) + mp_string(BIND_IP_TAG) + " " + CIP::ParseIPv6(strIP, true) +
                       CHAR_COLON + strLast;
        break;
    }

    iRet = CIPCFile::WriteFile(strNginxConfFile, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Write nginx config file failed, iRet = %d, size of vecResult is %d.",
            iRet,
            vecResult.size());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 获取当前主机所有ip地址
Return       : MP_SUCCESS -- 成功
------------------------------------------------------------- */
mp_int32 ChangeIP::GetLocalIPs(vector<mp_string>& vecIPs)
{
    // CodeDex误报，UNINIT
    mp_int32 iRet;
#ifdef WIN32
    iRet = GetLocalIPsWin(vecIPs);
#elif defined LINUX
    iRet = GetLocalIPsLinux(vecIPs);
#else
    iRet = GetLocalIPsOther(vecIPs);
#endif
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    vecIPs.emplace_back (ANY_IP);
    return MP_SUCCESS;
}

#ifdef WIN32
mp_int32 ChangeIP::GetAdapterAddr(PIP_ADAPTER_ADDRESSES& pCurrAddresses)
{
    mp_ulong outBufLen = 0;
    GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &outBufLen);
    if (outBufLen == 0) {
        COMMLOG(OS_LOG_ERROR, "GetAdaptersAddresses get len faild.");
        pCurrAddresses = NULL;
        return MP_FAILED;
    }

    PIP_ADAPTER_ADDRESSES pAddresses = static_cast<IP_ADAPTER_ADDRESSES*>(malloc(outBufLen));
    if (pAddresses == NULL) {
        COMMLOG(OS_LOG_ERROR, "GetAdaptersAddresses malloc buff faild.");
        pCurrAddresses = NULL;
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, "GetAdaptersAddresses outBufLen length = ", outBufLen);
    if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_ANYCAST, NULL, pAddresses, &outBufLen) != NO_ERROR) {
        COMMLOG(OS_LOG_ERROR, "GetAdaptersAddresses faild.");
        pCurrAddresses = NULL;
        free(pAddresses);
        pAddresses = NULL;
        return MP_FAILED;
    }

    pCurrAddresses = pAddresses;
    return MP_SUCCESS;
}

mp_int32 ChangeIP::GetLocalIPsWin(vector<mp_string>& vecIPs)
{
    mp_uint32 bufflen = 100;
    PIP_ADAPTER_ADDRESSES pCurrAddresses;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast;
    LPSOCKADDR addr = NULL;
    char buff[CHANGEIP_NUM_100] = {0};

    DWORD iRet = GetAdapterAddr(pCurrAddresses);
    if (iRet != MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "Get adapter address faild.");
        return iRet;
    }

    while (pCurrAddresses) {
        if (pCurrAddresses->OperStatus != IfOperStatusUp) {
            pCurrAddresses = pCurrAddresses->Next;
            continue;
        }
        pUnicast = pCurrAddresses->FirstUnicastAddress;

        while (pUnicast) {
            addr = pUnicast->Address.lpSockaddr;
            if (addr->sa_family == AF_INET6) {
                sockaddr_in6* sa_in6 = reinterpret_cast<sockaddr_in6*>(addr);
                inet_ntop(AF_INET6, &(sa_in6->sin6_addr), buff, bufflen);
            } else {
                sockaddr_in* sa_in = reinterpret_cast<sockaddr_in*>(addr);
                inet_ntop(AF_INET, &(sa_in->sin_addr), buff, bufflen);
            }
            pUnicast = pUnicast->Next;
            if ((strcmp(buff, LOCAL_IP_ADDRESS.c_str()) != 0) && (strcmp(buff, LOCAL_IP6_ADDRESS.c_str()) != 0)) {
                vecIPs.push_back(buff);
            }
        }
        pCurrAddresses = pCurrAddresses->Next;
    }

    free(pCurrAddresses);
    pCurrAddresses = NULL;

    return MP_SUCCESS;
}
#elif defined LINUX
mp_int32 ChangeIP::GetLocalIPsLinux(vector<mp_string>& vecIPs)
{
    struct ifaddrs* ifaddr = NULL;
    mp_int32 family = AF_INET;
    mp_int32 iRet = MP_FAILED;
    mp_char host[NI_MAXHOST] = {0};

    if (getifaddrs(&ifaddr) == -1) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[MAX_ERROR_MSG_LEN] = {0};
        COMMLOG(OS_LOG_ERROR, "getifaddrs failed, errno[%d]: %s", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            COMMLOG(OS_LOG_DEBUG, "ifa->ifa_addr == NULL");
            continue;
        }

        family = ifa->ifa_addr->sa_family;
        mp_bool bFlag = (family != AF_INET) && (family != AF_INET6);
        if (bFlag) {
            COMMLOG(OS_LOG_DEBUG, "family != AF_INET or family != AF_INET6");
            continue;
        }
        if (family == AF_INET) {
            iRet = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (iRet != 0) {
                COMMLOG(OS_LOG_INFO, "getnameinfo ipv4 failed, errno %s", gai_strerror(iRet));
                continue;
            }
        }
        if (AF_INET6 == family) {
            iRet = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (iRet != 0) {
                COMMLOG(OS_LOG_INFO, "getnameinfo ipv6 failed, errno %s", gai_strerror(iRet));
                continue;
            }
        }
        if (strcmp(host, LOCAL_IP_ADDRESS.c_str()) != 0) {
            vecIPs.push_back(host);
        }
    }
    freeifaddrs(ifaddr);

    return MP_SUCCESS;
}
#else
mp_int32 ChangeIP::GetLocalIPsOther(vector<mp_string>& vecIPs)
{
    mp_string strCMD;
#ifdef AIX
    if (CIP::isIPV4) {
        strCMD = "ifconfig -a|grep \"inet \"|awk '{print $2}'";
    } else {
        strCMD = "ifconfig -a|grep \"inet6 \"|awk '{print $2}' |awk -F'/' '{print $1}' | grep -v '^:'";
    }
#elif defined HP_UX_IA
    if (CIP::isIPV4) {
        strCMD = "netstat -ni|grep -v 'Address'|awk '{print $4}'";
    } else {
        strCMD = "line=`netstat -ni | sed -n -e '/Address\\/Prefix/='`;netstat -ni | sed -n $line',$p' |awk -F '/' "
                 "'{print $1}' | awk '{print $3}'";
    }
#elif defined SOLARIS
    strCMD = "netstat -ni | grep -v 'Address' | sed '$d' | nawk '{print $4}'";
#endif
    vector<mp_string> tmpIPs;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCMD, tmpIPs);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get all ip failed, iRet = %d and ipcount = %d", iRet, tmpIPs.size());
        return MP_FAILED;
    }
    for (vector<string>::iterator iter_tmp = tmpIPs.begin(); iter_tmp != tmpIPs.end(); iter_tmp++) {
        if (*iter_tmp != LOCAL_IP_ADDRESS) {
            vecIPs.push_back(*iter_tmp);
        }
    }

    return MP_SUCCESS;
}
#endif

/* ------------------------------------------------------------
Description  : 判断当前ip是否是本地ip
Return       : MP_SUCCESS -- 成功
------------------------------------------------------------- */
mp_bool ChangeIP::IsLocalIP(const mp_string& strIP)
{
    vector<mp_string> vecIPs;
    mp_int32 iRet = GetLocalIPs(vecIPs);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetLocalIPs failed, iRet = %d", iRet);
        return MP_FALSE;
    }
    for (vector<mp_string>::iterator it = vecIPs.begin(); it != vecIPs.end(); ++it) {
        COMMLOG(OS_LOG_DEBUG, "%s", it->c_str());
    }
    for (vector<mp_string>::iterator it = vecIPs.begin(); it != vecIPs.end(); ++it) {
        if (strIP == *it) {
            return MP_TRUE;
        }
    }

    return MP_FALSE;
}

mp_int32 ChangeIP::RestartNginx()
{
#ifdef WIN32
    mp_string strCmd = mp_string("sc stop ") + NGINX_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    strCmd = mp_string("sc start ") + NGINX_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    // 校验脚本签名
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_AS_PARAM_NAME;
    if (getuid() == 0) {
        strCmd = "su - rdadmin -c \" " + strCmd + " \" ";
    }
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    COMMLOG(OS_LOG_DEBUG, "execute :%s", strCmd.c_str());
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd = strCmd + " " + NGINX_AS_PARAM_NAME;
    if (getuid() == 0) {
        strCmd = "su - rdadmin -c \" " + strCmd + " \" ";
    }
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    COMMLOG(OS_LOG_DEBUG, "execute :%s", strCmd.c_str());
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif
    return MP_SUCCESS;
}
