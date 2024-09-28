#include <string>
#include "common/Defines.h"
#include "common/Log.h"
#include "common/JsonUtils.h"
#include "common/Utils.h"
#include "securec.h"
#include "securecom/CryptAlg.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/Pipe.h"

#ifndef WIN32
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
using namespace std;

namespace PIPE_PARAM {
const mp_int32 BUFFER_SIZE = 1023;
const mp_int32 READPIPE_WAITTIME = 1000;
}  // namespace PIPE_PARAM
CMpPipe::CMpPipe()
{
#ifdef WIN32
    m_hPipe = NULL;
#else
    m_fd = 0;
#endif  // WIN32
    std::atomic_init(&m_bOperateEnd, false);
    m_strUniqueID = "";
    m_strInput = "";
    m_vecInput.clear();
}
mp_int32 CMpPipe::ReadPipe(const mp_string& strFileName, std::vector<mp_string>& vecOutput)
{
    LOGGUARD("strFileName is %s.", strFileName.c_str());
    mp_char szErr[256] = {0};
    mp_int32 iErr = 0;
    mp_int32 iRet = OpenPipe(strFileName, true);
    if (iRet != MP_SUCCESS) {
        iErr = GetOSError();
        COMMLOG(
            OS_LOG_ERROR, "Open Pipe for read failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    unsigned long retRead = 0;
    do {
        mp_char cBuff[PIPE_PARAM::BUFFER_SIZE + 1] = {0};
#ifdef WIN32
        ReadFile(m_hPipe, cBuff, PIPE_PARAM::BUFFER_SIZE, &retRead, NULL);
#else
        retRead = read(m_fd, cBuff, PIPE_PARAM::BUFFER_SIZE);
#endif  // WIN32
        if (retRead == 0) {
            CMpTime::DoSleep(PIPE_PARAM::READPIPE_WAITTIME);
            continue;
        } else if (retRead < 0) {
            iErr = GetOSError();
            COMMLOG(OS_LOG_ERROR, "read pipe failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            ClosePipe();
            UnlinkPipe(strFileName);
            return MP_FAILED;
        }
        vecOutput.push_back(cBuff);
    } while (retRead > 0);
    ClosePipe();
    return MP_SUCCESS;
}

mp_int32 CMpPipe::WritePipe(const mp_string& strFileName, vector<mp_string>& vecInput)
{
    LOGGUARD("strFileName is %s", strFileName.c_str());
    mp_char szErr[256] = {0};
    mp_int32 iErr = 0;
    mp_int32 iRet = OpenPipe(strFileName, false);
    if (iRet != MP_SUCCESS) {
        iErr = GetOSError();
        COMMLOG(
            OS_LOG_ERROR, "Open Pipe for write failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return iRet;
    }
    for (vector<mp_string>::iterator iter = vecInput.begin(); iter != vecInput.end(); ++iter) {
        int nByteWrite = 0;
        iRet = WritePipeInner(strFileName, *iter, nByteWrite);
        if (iRet != MP_SUCCESS) {
            iErr = GetOSError();
            COMMLOG(
                OS_LOG_ERROR, "WritePipeInner failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            ClosePipe();
            UnlinkPipe(strFileName);
            return iRet;
        }
        ClearString(*iter);
    }
    ClosePipe();
    m_vecInput.clear();
    ClearString(m_strInput);
    // 管道同步读写，部分脚本读完没有删除，故在此删除，避免重启进程后下次没有参数的脚本读到同一个id的文件卡住
    iRet = UnlinkPipe(strFileName);
    if (iRet != MP_SUCCESS) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Unlink Pipe failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CMpPipe::WritePipeInner(const mp_string& strFileName, const mp_string& strInput, int& nByteWrite)
{
    mp_char szErr[256] = {0};
    mp_int32 iErr = 0;
#ifdef WIN32
    typedef unsigned Bool;
    mp_bool bRet = WriteFile(m_hPipe, strInput.c_str(), (DWORD)strlen(strInput.c_str()), (LPDWORD)&nByteWrite, NULL);
    if (!bRet) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "write pipe failed, byteWrite %d, errno[%d]:%s.",
            nByteWrite,
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
#else
    nByteWrite = write(m_fd, strInput.c_str(), strlen(strInput.c_str()));
#endif  // WIN32
    if (nByteWrite < 0) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR,
            "write pipe failed, byteWrite %d, errno[%d]:%s.",
            nByteWrite,
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
#ifdef WIN32
    DisconnectNamedPipe(m_hPipe);
#endif
    return MP_SUCCESS;
}

mp_int32 CMpPipe::OpenPipe(const mp_string& strFileName, mp_bool bReader)
#ifdef WIN32
{
    mp_char szErr[256] = {0};
    mp_int32 iErr = 0;
    if (bReader) {
        m_hPipe = CreateNamedPipe(strFileName.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            0,
            0,
            NMPWAIT_WAIT_FOREVER,
            0);
        if (m_hPipe == NULL) {
            iErr = GetOSError();
            COMMLOG(OS_LOG_ERROR, "create pipe failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            UnlinkPipe(strFileName);
            return MP_FAILED;
        }
        if (!ConnectNamedPipe(m_hPipe, NULL)) {
            iErr = GetOSError();
            COMMLOG(OS_LOG_ERROR, "connect pipe failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            UnlinkPipe(strFileName);
            return MP_FAILED;
        }
    } else {
        if (!WaitNamedPipe(strFileName.c_str(), NMPWAIT_WAIT_FOREVER)) {
            iErr = GetOSError();
            COMMLOG(OS_LOG_ERROR, "connect pipe failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            UnlinkPipe(strFileName);
            return MP_FAILED;
        }
        m_hPipe = CreateFile(strFileName.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (m_hPipe == NULL) {
            iErr = GetOSError();
            COMMLOG(
                OS_LOG_ERROR, "create pipefile failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            UnlinkPipe(strFileName);
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}
#else
{
    if (bReader) {
        m_fd = open(strFileName.c_str(), O_RDONLY);
    } else {
        m_fd = open(strFileName.c_str(), O_WRONLY);
    }
    if (m_fd < 0) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, "open pipe failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        ClosePipe();
        UnlinkPipe(strFileName);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
#endif  // WIN32
mp_void CMpPipe::ClosePipe()
{
#ifdef WIN32
    CloseHandle(m_hPipe);
#else
    close(m_fd);
#endif  // WIN32
}

#ifndef WIN32
mp_int32 CMpPipe::CreatePipe(const mp_string& strFileName)
{
    if (CMpFile::FileExist(strFileName) != MP_TRUE) {
        int retMk = mkfifo(strFileName.c_str(), 0600);
        if (retMk != 0) {
            mp_int32 iErr = GetOSError();
            mp_char szErr[256] = {0};
            COMMLOG(OS_LOG_ERROR, "create pipe failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            UnlinkPipe(strFileName);
            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}
#endif  // WIN32

mp_int32 CMpPipe::UnlinkPipe(const mp_string& strPipeName)
{
    if (strPipeName == "") {
        return MP_SUCCESS;
    }
    if (CMpFile::FileExist(strPipeName) != MP_TRUE) {
        return MP_SUCCESS;
    }
    if (remove(strPipeName.c_str()) != MP_SUCCESS) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR,
            "Remove file %s failed, errno[%d]:%s.",
            strPipeName.c_str(),
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CMpPipe::WriteInput(const mp_string& strUniqueID, const mp_string& strInput)
{
    LOGGUARD("Unique ID is %s", strUniqueID.c_str());
    mp_string strFileName = INPUT_TMP_FILE + strUniqueID;
    // 增加临时文件夹路径
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(std::move(strFileName));
    vector<mp_string> vecInput;
    vecInput.push_back(strInput);
    mp_int32 iRet = WritePipe(strFilePath, vecInput);
    if (iRet != MP_SUCCESS) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(
            OS_LOG_ERROR, "Write input info file failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_void CMpPipe::SetUniqueID(const mp_string& strUniqueID)
{
    m_strUniqueID = strUniqueID;
}

const mp_string& CMpPipe::GetUniqueID()
{
    return m_strUniqueID;
}

mp_void CMpPipe::SetStrInput(const mp_string& strInput)
{
    m_strInput = strInput;
}

const mp_string& CMpPipe::GetStrInput()
{
    return m_strInput;
}

mp_void CMpPipe::SetVecInput(std::vector<mp_string>& vecInput)
{
    m_vecInput = vecInput;
}

mp_void CMpPipe::GetVecInput(std::vector<mp_string>& vecInput)
{
    vecInput = m_vecInput;
}

mp_int32 CMpPipe::ReadInput(const mp_string& strUniqueID, mp_string& strInput)
{
    LOGGUARD("Unique ID is %s", strUniqueID.c_str());
    mp_string strFileName = INPUT_TMP_FILE + strUniqueID;
    // 增加临时文件夹路径
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(std::move(strFileName));
    vector<mp_string> vecOutput;
    mp_int32 iRet = ReadPipe(strFilePath, vecOutput);
    if (MP_SUCCESS != iRet) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(
            OS_LOG_ERROR, "Read input from pipe failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return iRet;
    }

    if (vecOutput.size() != 1) {
        COMMLOG(OS_LOG_ERROR, "Invalid file content, ret %d.", vecOutput.size());
        return MP_FAILED;
    }

    strInput = vecOutput[0];
    COMMLOG(OS_LOG_DEBUG, "Read input from pipe succ.");
    return MP_SUCCESS;
}
