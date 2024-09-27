#include "securecom/Ip.h"
#include "common/Log.h"
#include "message/tcp/CSocket.h"

using namespace std;

namespace SecureCom {
mp_bool CIP::CheckHostLinkStatus(const mp_string& strSrcIP, const std::vector<mp_string>& hostIpList)
{
    static const mp_int32 checkNfsPort = 111;
    COMMLOG(OS_LOG_DEBUG,
        "Will check host link status, strSrcIP[%s] strHost.size(%d)",
        strSrcIP.c_str(),
        hostIpList.size());

    mp_bool bRet = MP_FALSE;
    mp_string tHostIp;
    std::vector<mp_string>::const_iterator it = hostIpList.begin();
    for (; it != hostIpList.end(); ++it) {
        tHostIp = *it;
        if (tHostIp.empty()) {
            continue;
        }

        if (CSocket::CheckHostLinkStatus(strSrcIP, tHostIp, checkNfsPort) == MP_SUCCESS) {
            COMMLOG(OS_LOG_DEBUG, "src[%s] link dst[%s] success.", strSrcIP.c_str(), tHostIp.c_str());
            bRet = MP_TRUE;
            break;
        } else {
            COMMLOG(OS_LOG_ERROR, "src[%s] link dst[%s] failed, will test next ip.", strSrcIP.c_str(), tHostIp.c_str());
        }
    }

    if (bRet == MP_FALSE) {
        COMMLOG(OS_LOG_ERROR, "src[%s] link all HostIP failed.", strSrcIP.c_str());
    } else {
        COMMLOG(OS_LOG_DEBUG, "src[%s] link success.", strSrcIP.c_str());
    }

    return bRet;
}

mp_int32 CIP::CheckHostLinkStatus(const std::vector<mp_string>& hostIpv4List,
    const std::vector<mp_string>& hostIpv6List, std::vector<mp_string>& srcIpv4List,
    std::vector<mp_string>& srcIpv6List)
{
    mp_int32 iRet = MP_SUCCESS;

    if (hostIpv4List.size() == 0 && hostIpv6List.size() == 0) {
        COMMLOG(OS_LOG_INFO, "Host ip list is empty, return all found ips.");
        return iRet;
    }

    std::vector<mp_string> tIpv4List;
    std::vector<mp_string> tIpv6List;

    tIpv4List.swap(srcIpv4List);
    tIpv6List.swap(srcIpv6List);

    mp_string tStrIp;

    if (hostIpv4List.size() > 0) {
        vector<mp_string>::iterator itV4;
        for (itV4 = tIpv4List.begin(); itV4 != tIpv4List.end(); ++itV4) {
            tStrIp = *itV4;
            if (MP_TRUE != CheckHostLinkStatus(tStrIp, hostIpv4List)) {
                COMMLOG(OS_LOG_ERROR, "checkHostLinkStatus failed, src[%s] link failed.", tStrIp.c_str());
                continue;
            }

            srcIpv4List.push_back(tStrIp);
        }
    }

    if (hostIpv6List.size() > 0) {
        vector<mp_string>::iterator itV6;
        for (itV6 = tIpv6List.begin(); itV6 != tIpv6List.end(); ++itV6) {
            tStrIp = *itV6;
            if (MP_TRUE != CheckHostLinkStatus(tStrIp, hostIpv6List)) {
                COMMLOG(OS_LOG_ERROR, "checkHostLinkStatus failed, src[%s] link failed.", tStrIp.c_str());
                continue;
            }

            srcIpv6List.push_back(tStrIp);
        }
    }

    return iRet;
}
}  // namespace SecureCom
