/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include <fstream>
#include "common/LogRotater.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/File.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "common/SecureCmdExecutor.h"

namespace {
const mp_int32 ROTATE_TIME = 5;
}

LogRotater LogRotater::m_instance;
LogRotater& LogRotater::GetInstance()
{
    return m_instance;
}

LogRotater::LogRotater()
{
    m_iMaxSize = DEFAULT_LOG_SIZE;
    m_iLogCount = DEFAULT_LOG_COUNT;
    m_iLogKeepTime = DEFAULT_LOG_KEEP_TIME;
}

LogRotater::~LogRotater()
{
    if (m_logRotateThread != nullptr) {
        m_stopFlag.store(true);
        m_logRotateThreadCV.notify_one();
        m_logRotateThread->join();
        m_logRotateThread.reset();
    }
}

mp_void LogRotater::Init(const mp_string& strFilePath, const mp_string& strFileName)
{
    m_strFilePath = strFilePath;
    m_strFileName = strFileName;
    if (m_logRotateThread == nullptr) {
        m_logRotateThread = std::make_unique<std::thread>(std::bind(&LogRotater::LogRotateThread, this));
    }
}

mp_void LogRotater::LogRotateThread()
{
    while (!m_stopFlag.load()) {
        std::unique_lock<std::mutex> lk(m_logRotateThreadCVLock);
        m_logRotateThreadCV.wait_for(lk, std::chrono::minutes(ROTATE_TIME),
            [this]() { return this->m_stopFlag.load(); });
        m_iMaxSize = CLogger::GetInstance().GetLogMaxSize();
        m_iLogCount = CLogger::GetInstance().GetLogCount();
        m_iLogKeepTime = CLogger::GetInstance().GetLogKeepTime();
        COMMLOG(OS_LOG_INFO, "Logmaxsize is \"%d\", logcount is \"%d\", logkeeptime is \"%d\"",
            m_iMaxSize, m_iLogCount, m_iLogKeepTime);
        LogRotate();
    }
}

mp_int32 LogRotater::HandlerAccess(mp_string &strFileName)
{
    mp_string oldstrLogFile = mp_string("") + m_strFilePath + PATH_SEPARATOR + strFileName;
    strFileName += ".bak";
    mp_string newstrLogFile = mp_string("") + m_strFilePath + PATH_SEPARATOR + strFileName;
    (void)rename(oldstrLogFile.c_str(), newstrLogFile.c_str());

    mp_uint32 uiRetCode = 0;
#ifdef WIN32
    mp_string strCommand = CPath::GetInstance().GetBinFilePath(AGENTCLI_EXE);
    strCommand = CMpString::BlankComma(strCommand);
    strCommand = strCommand + " " + NGINX_RELOAD;
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCommand);
#else
    mp_string strCommand = CPath::GetInstance().GetBinFilePath(AGENTCLI_UNIX);
    strCommand = CMpString::BlankComma(strCommand);
    mp_int32 iRet = SecureCmdExecutor::ExecuteWithoutEcho("%s %s", {strCommand, NGINX_RELOAD});
#endif
    return iRet;
}

mp_int32 LogRotater::LogRotateInner()
{
    mp_string strFileName = m_strFileName;
    mp_int32 iRet = HandlerAccess(strFileName);
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_WARN, "Log file rotate failed before switch.");
        return iRet;
    }
    iRet = CLogger::GetInstance().SwitchLogFile(m_strFilePath, strFileName, m_iLogCount, m_iLogKeepTime);
    mp_string strLogFile = m_strFilePath + PATH_SEPARATOR + strFileName;
    (void)remove(strLogFile.c_str());
    return iRet;
}

 /* ------------------------------------------------------------
Description  : 对指定位置的日志进行转储
Return       : MP_SUCCESS -- 成功
------------------------------------------------------------- */
mp_int32 LogRotater::LogRotate()
{
    COMMLOG(OS_LOG_INFO, "Log file path is \"%s\", file name is \"%s\"", m_strFilePath.c_str(), m_strFileName.c_str());
    mp_string strLogFilePath = m_strFilePath + PATH_SEPARATOR + m_strFileName;
    mp_int32 iRet = 0;
    FILE* pFile = NULL;
#ifdef WIN32
    mp_uint32 uiFileSize = 0;

    if (MP_TRUE != CMpFile::FileExist(strLogFilePath)) {
        COMMLOG(OS_LOG_WARN, "Log file path not exist, path is \"%s\"", strLogFilePath.c_str());
        return MP_FAILED;
    }

    if (CMpFile::FileSize(strLogFilePath.c_str(), uiFileSize) != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Log file size can not get, path is \"%s\"", strLogFilePath.c_str());
        return MP_FAILED;
    }

    if (uiFileSize >= (mp_uint32)m_iMaxSize) {
        iRet = LogRotateInner();
    }
#else
    mp_int32 fd;
    struct stat st;

    fd = open(strLogFilePath.c_str(), O_RDONLY);
    if (-1 == fd) {
        COMMLOG(OS_LOG_WARN, "Log file path not exist, path is \"%s\"", strLogFilePath.c_str());
        return MP_FAILED;
    }
    (void)fstat(fd, &st);
    close(fd);
    if (st.st_size >= m_iMaxSize) {
        iRet = LogRotateInner();
    }
#endif
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Log file %s rotate failed.", strLogFilePath.c_str());
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_INFO, "Log file %s has been succesfully rotated.", strLogFilePath.c_str());
    return MP_SUCCESS;
}