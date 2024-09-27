#ifndef __AGENT_TSF_TIMEOUT_H__
#define __AGENT_TSF_TIMEOUT_H__

#include "common/Defines.h"
#include "common/Types.h"

#define TIMEOUT_INFINITE ((mp_uint32)-1)

class AGENT_API CTimeOut {
public:
    CTimeOut(mp_uint32 uiSecTimeOut);
    ~CTimeOut();

    mp_uint32 Remaining();

private:
    mp_time m_tStart;
    mp_uint32 m_uiSecTimeout;
};

#endif  // __AGENT_TSF_TIMEOUT_H__
