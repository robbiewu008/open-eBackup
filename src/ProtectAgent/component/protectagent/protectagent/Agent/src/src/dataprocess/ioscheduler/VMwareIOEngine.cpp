#include "dataprocess/ioscheduler/VMwareIOEngine.h"
#include "common/Log.h"

mp_int32 VMwareIOEngine::Read(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer)
{
    if (m_vixDiskApi == NULL) {
        COMMLOG(OS_LOG_ERROR, "Disk handle is null");
        return MP_FAILED;
    }

    int ret = m_vixDiskApi->Read(offsetInBytes, bufferSizeInBytes, buffer, m_errDesc);
    if (ret != VIX_OK) {
        COMMLOG(OS_LOG_ERROR, "Read failed, err: '%s'", m_errDesc.c_str());
        return ret;
    }

    return MP_SUCCESS;
}

mp_int32 VMwareIOEngine::Write(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer)
{
    if (m_vixDiskApi == NULL) {
        COMMLOG(OS_LOG_ERROR, "Disk handle is null");
        return MP_FAILED;
    }

    int ret = m_vixDiskApi->Write(offsetInBytes, bufferSizeInBytes, buffer, m_errDesc);
    if (ret != VIX_OK) {
        COMMLOG(OS_LOG_ERROR, "Write failed, err: '%s'", m_errDesc.c_str());
        return ret;
    }

    return MP_SUCCESS;
}
