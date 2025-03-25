#ifndef FILE_H_
#define FILE_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <mutex>
#include <common/CMpTime.h>
#include "xbsa/xbsa.h"
#include "message/archivestream/ArchiveStreamService.h"

struct BsaObjContext {
    std::string storgePath;
    int getDataType{0};
    std::string archiveBackupId;
    std::string archiveServerIp;
    std::string fsID;
    int archiveServerPort{0};
    int archiveOpenSSL{0}; // 1:open  0:close
    int status{0}; // 1:open  0:close
    ArchiveStreamGetFileRsq stFileInfo;
    BSA_ObjectStatus objectStatus = BSA_ObjectStatus_ANY;
};

struct BsaHandleContext {
    ArchiveStreamService streamService;
    BsaObjContext workingObj; // the object that's reading or writing.
    std::map<std::string, BsaObjContext> queryMap; // all objects had queried from server.
};

enum class FileIoStatus {
    CLOSE = 0,
    OPEN = 1,
};

class File {
public:
    File();
    ~File();

    int OpenForRead(long bsaHandle, const std::string &storgePath);
    int OpenForWrite(long bsaHandle, const std::string &storgePath);
    void Close();
    int Read(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);
    int ConnectArchive(long bsaHandle, BsaHandleContext &context);
    EXTER_ATTACK int ReadFromArchive(long bsaHandle, BSA_DataBlock32 *dataBlockPtr, BsaHandleContext &context);
    int Write(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);
    void SetWriteStatus(const FileIoStatus &status);
    void SetReadStatus(const FileIoStatus &status);
    FileIoStatus GetWriteStatus();
    FileIoStatus GetReadStatus();
    time_t GetLastTime();
    void UpdateLastTime();
    time_t GetNowTime();

    int OpenForWriteWithSameFile(long bsaHandle, const std::string &storgePath, mp_string writeType);

    int CreateFileWhenFileNotExist(const std::string &pszFilePath, mp_string writeType);

private:
    int FileExist(const std::string &pszFilePath);
    int CreateFile(const std::string &pszFilePath);
    int ExecSystemCmd(const std::string& strCommand, std::vector<std::string>& strEcho);

private:
    FILE* m_fp = NULL;
    FileIoStatus m_writeStatus = FileIoStatus::CLOSE; // 写数据状态 0-关闭， 1-打开
    FileIoStatus m_readStatus = FileIoStatus::CLOSE; // 读数据状态 0-关闭， 1-打开
    time_t m_lastTime = 0; // 上一次给XBSA Server发送SendData或GetData的时间
};

#endif