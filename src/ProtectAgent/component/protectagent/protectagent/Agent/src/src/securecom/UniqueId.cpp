#include "securecom/UniqueId.h"
#include <sstream>
#include "common/CMpTime.h"
#include "securecom/CryptAlg.h"
using namespace std;
CUniqueID CUniqueID::m_instance;
CUniqueID& CUniqueID::GetInstance()
{
    return m_instance;
}
/* ------------------------------------------------------------
Function Name: GetString
Description  : 获取全局唯一ID，格式为string
Others       :-------------------------------------------------------- */
mp_string CUniqueID::GetString()
{
    CThreadAutoLock lock(&m_uniqueIDMutex);
    ostringstream oss;
    oss << GetTimestamp() <<"_";
    GetRandom(m_iUniqueID, false);
    oss << m_iUniqueID;
    return oss.str();
}

/* ------------------------------------------------------------
Function Name: GetInt
Description  : 获取全局唯一ID，格式为int
Others       :-------------------------------------------------------- */
mp_ulong CUniqueID::GetInt()
{
    CThreadAutoLock lock(&m_uniqueIDMutex);
    GetRandom(m_iUniqueID, false);
    return(mp_ulong)m_iUniqueID;
}

mp_string CUniqueID::GetTimestamp()
{
    mp_tm stCurTime;
    mp_time tmpTime;
    CMpTime::Now(tmpTime);
    mp_tm* pTime = CMpTime::LocalTimeR(tmpTime, stCurTime);
    if (pTime != nullptr) {
        std::ostringstream strBuf;
        strBuf << (pTime->tm_year + DATE_YEAR_1900) << (pTime->tm_mon + DATE_MONTH_1)
        << pTime->tm_mday << pTime->tm_sec;
        return strBuf.str();
    }
    return "";
}