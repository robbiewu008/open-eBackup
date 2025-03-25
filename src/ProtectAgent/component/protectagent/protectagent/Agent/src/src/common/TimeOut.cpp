#include "common/TimeOut.h"
#include "common/Log.h"
#include "common/CMpTime.h"

CTimeOut::CTimeOut(mp_uint32 uiSecTimeOut)
{
    CMpTime::Now(m_tStart);
    m_uiSecTimeout = uiSecTimeOut;
}

CTimeOut::~CTimeOut()
{
}

mp_uint32 CTimeOut::Remaining()
{
    COMMLOG(OS_LOG_DEBUG, "Begin calculate remainning time.");
    if (m_uiSecTimeout == 0) {
        COMMLOG(OS_LOG_DEBUG, "Remainning time is 0.");
        return 0;
    }

    if (TIMEOUT_INFINITE == m_uiSecTimeout) {
        COMMLOG(OS_LOG_DEBUG, "Remainning time is infinite.");
        return m_uiSecTimeout;
    }

    mp_time tNow;
    mp_uint32 uiRemain = 0;
    CMpTime::Now(tNow);
    mp_uint32 uiElapsed = tNow - m_tStart;
    if (uiElapsed >= m_uiSecTimeout) {
        COMMLOG(OS_LOG_DEBUG, "End calculate remainning time, uiRemain %d.", uiRemain);
        return 0;
    }

    uiRemain = m_uiSecTimeout - uiElapsed;
    COMMLOG(OS_LOG_DEBUG, "End calculate remainning time, uiRemain %d.", uiRemain);
    return uiRemain;
}

