#ifndef _AGENTCLI_TESTHOST_H_
#define _AGENTCLI_TESTHOST_H_

#include "common/Types.h"

class TestHost {
public:
    static mp_int32 Handle(const mp_string& strHostIp, const mp_string& strHostPort, const mp_string& strTimeout);
};

#endif
