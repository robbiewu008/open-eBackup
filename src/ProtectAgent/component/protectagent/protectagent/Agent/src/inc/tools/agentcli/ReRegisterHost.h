#ifndef _RE_REGISTER_HOST_H_
#define _RE_REGISTER_HOST_H_

#include "common/Types.h"
#include "common/Utils.h"

class ReRegisterHost {
public:
    ReRegisterHost() { }
    ~ReRegisterHost() { }
    mp_int32 Handle();

private:
    mp_int32 InitParam();
    mp_int32 GenerateAgentIP(bool bUseManager);
    mp_int32 GetChoice();
    mp_int32 SetNewNetParam();
    mp_void RollbackNetParam();

private:
    std::vector<mp_string> m_srcIpv4List;
    std::vector<mp_string> m_srcIpv6List;

    mp_string m_pmIpList;
    mp_int32 m_pmPort = 25082;
    mp_string m_pmManagerIpList;
    mp_int32 m_pmManagerPort = 25081;
    bool m_bUseManager = false;
    mp_string m_nginxIp;

    mp_string m_oldPmIpList;
    mp_int32 m_oldPmPort = 25082;
    mp_string m_oldNginxIp;
    mp_int32 m_oldNginxPort = 59526;
};

#endif