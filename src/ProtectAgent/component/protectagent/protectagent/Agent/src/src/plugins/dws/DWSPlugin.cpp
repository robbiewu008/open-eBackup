#include "plugins/dws/DWSPlugin.h"
#include <fstream>
#include <sstream>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/JsonHelper.h"
#include "apps/dws/XBSAServer/CTimer.h"
#include "apps/dws/XBSAServer/ThriftServer.h"
#include "apps/dws/XBSAServer/BsaSessionManager.h"

namespace {
    const mp_uint32 MB_TO_BYTE = 1024 * 1024;
    const mp_uint32 SCHEDULE_INTERVAL = 15 * 1000; // 15 seconds.
    const mp_uint32 ONE_DAY_TIME = 24 * 60 * 60;
    mp_uint32 RETRY_TIMES = 30;
    const mp_uint32 RETRY_TIMES_INTERVAL = 10*1000;
}

REGISTER_PLUGIN(DWSPlugin);
DWSPlugin::DWSPlugin() : m_schedulethreadExitFlag(MP_FALSE), m_scheduleInterval(SCHEDULE_INTERVAL)
{
    (mp_void)memset_s(&m_scheduleThreadId, sizeof(m_scheduleThreadId), 0, sizeof(m_scheduleThreadId));
}

DWSPlugin::~DWSPlugin()
{
    m_schedulethreadExitFlag = MP_TRUE;
    if (m_scheduleThreadId.os_id != 0) {
        CMpThread::WaitForEnd(&m_scheduleThreadId, NULL);
    }
}

mp_int32 DWSPlugin::DoAction(CDppMessage &reqMsg, CDppMessage &rspMsg)
{
    DO_DPP_ACTION(DWSPlugin, reqMsg, rspMsg);
}

mp_int32 DWSPlugin::Init(std::vector<mp_uint32> &cmds)
{
    mp_int32 iRet = MP_SUCCESS;
    while (RETRY_TIMES--) {
        iRet = ThriftServer::GetInstance()->Init();
        if (iRet != MP_SUCCESS) {
            INFOLOG("Init ThriftServer failed, retry.");
            CMpTime::DoSleep(RETRY_TIMES_INTERVAL);
            continue;
        }
        break;
    }
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init ThriftServer failed.");
        return MP_FAILED;
    }

    iRet = CTimer::GetInstance().Init();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init CTimer failed.");
        return MP_FAILED;
    }

    iRet = CMpThread::Create(&m_scheduleThreadId, ScheduleThread, this);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init send task worker failed, errno[%d].", iRet);
        return MP_FAILED;
    }

    INFOLOG("Init DWSPlugin success.");
    return MP_SUCCESS;
}

#ifdef WIN32
DWORD WINAPI DWSPlugin::ScheduleThread(mp_void *object)
#else
void *DWSPlugin::ScheduleThread(mp_void *object)
#endif
{
    DWSPlugin *tplugin = static_cast<DWSPlugin *>(object);
    if (tplugin == NULL) {
        ERRLOG("convert to DWSPlugin failed.");
#ifdef WIN32
        return MP_SUCCESS;
#else
        return NULL;
#endif
    }

    tplugin->ScheduleThreadRun();

#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

mp_void DWSPlugin::ScheduleThreadRun()
{
    uint64_t lastTotalSize = 0;
    mp_string lastCachePath;

    while (!m_schedulethreadExitFlag) {
        CMpTime::DoSleep(m_scheduleInterval);
        std::map<mp_string, taskInfo> taskDataSizeList = BsaSessionManager::GetInstance().GetTaskDataSize();
        for (auto &iter : taskDataSizeList) {
            DwsCacheInfo cacheInfo;
            if (BsaSessionManager::GetInstance().GetTaskCacheInfo(iter.first, cacheInfo) != MP_SUCCESS) {
                continue;
            }
            std::time_t taskStartTime = iter.second.startTime;
            std::time_t currentTime = std::time(nullptr);
            if (static_cast<int>(std::difftime(currentTime, taskStartTime))>=ONE_DAY_TIME) {
                BsaSessionManager::GetInstance().ClearTaskDataSizeByTaskId(iter.first);
            }
        }
    }
}

mp_void DWSPlugin::WriteSpeedFile(uint64_t totalSize, const std::string &output, const DwsCacheInfo &cacheInfo)
{
    mp_string hostKey = cacheInfo.hostKey;
    mp_string speedFile = cacheInfo.cacheRepoPath + "/tmp/"
        + cacheInfo.copyId + "/speed/" + hostKey + "/xbsa_speed.txt";
    std::ofstream outfile(speedFile, std::ios::trunc | std::ios_base::binary);
    if (!outfile.is_open()) {
        // speed file directory will be deleted in post job,so this could happen in normal case.
        WARNLOG("Open speed file(%s) failed!errno=%d.", speedFile.c_str(), errno);
        return;
    }
    outfile.write(output.c_str(), output.length());
    if (outfile.fail()) {
        ERRLOG("Write speed file(%s) fail!totalSizeInMB=%llu,errno=%d.", speedFile.c_str(), totalSize, errno);
    } else {
        INFOLOG("Write speed file(%s) success!totalSizeInMB=%llu.", speedFile.c_str(), totalSize);
    }
}
