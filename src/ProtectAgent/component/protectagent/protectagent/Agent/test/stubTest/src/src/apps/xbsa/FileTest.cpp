#include "apps/xbsa/FileTest.h"
#include "xbsaclientcomm/ThriftClientMgr.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"

int flag;
namespace {
mp_void LogReturn(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...)
{
    return;
}

int FileExistFailed(const std::string pszFilePath)
{
    return MP_FALSE;
}

int FileExistSuccess(const std::string pszFilePath)
{
    return MP_TRUE;
}

int CreateFileFailed(const std::string pszFilePath)
{
    return MP_FAILED;
}

int CreateFileSuccess(const std::string pszFilePath)
{
    return MP_SUCCESS;
}

FILE *fopenNull(const char *__restrict __filename,const char *__restrict __modes)
{
    return nullptr;
}

FILE *fopenNoNull(const char *__restrict __filename,const char *__restrict __modes)
{
    return (new FILE);
}

int statFailed(const char *__restrict __file, struct stat *__restrict __buf)
{
    return -1;
}

int statSuccess(const char *__restrict __file, struct stat *__restrict __buf)
{
    __buf->st_mode = 0;
    return 0;
}

int statSuccessIsDir(const char *__restrict __file, struct stat *__restrict __buf)
{
    __buf->st_mode = 0x4000;
    return 0;
}

int ExecSystemCmdFailed(const std::string& strCommand, std::vector<std::string>& strEcho)
{
    return MP_FAILED;
}

int ExecSystemCmdSuccess(const std::string& strCommand, std::vector<std::string>& strEcho)
{
    return MP_SUCCESS;
}

size_t freadSuccess(void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream)
{
    return 1;
}

FILE *popenNull(const char *__command, const char *__modes)
{
    return nullptr;
}

FILE *Stubpopen(const char *__command, const char *__modes)
{
    FILE ret;
    return &ret;
}

FileIoStatus GetWriteStatusClose()
{
    return FileIoStatus::CLOSE;
}

FileIoStatus GetWriteStatusOpen()
{
    return FileIoStatus::OPEN;
}

FileIoStatus GetReadStatusClose()
{
    return FileIoStatus::CLOSE;
}

FileIoStatus GetReadStatusOpen()
{
    return FileIoStatus::OPEN;
}

int StubSuccess()
{
    return MP_SUCCESS;
}

int StubSuccessOnTwo()
{
    if (flag++ < 1){
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int StubFailed()
{
    return MP_FAILED;
}
bool StubFileExist(const std::string &pszFilePath)
{
    return MP_TRUE;
}

int StubCreateFileFailed(const std::string &pszFilePath){
    MP_FAILED;
}

size_t StubFreadOrFwrite()
{
    size_t ret = 0;
    return ret;
}
}

/*
* 测试用例：OpenForWrite接口测试
* 前置条件：无
* CHECK点：1.写状态不为0时返回成功,2.写状态为0时创建文件成功.
*/
TEST_F(FileTest, OpenForWriteTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    File fileIO;
    long bsaHandle = 0;
    const std::string storgePath = "/tmp/OpenForWriteTest";

    stub.set(ADDR(File, GetWriteStatus), GetWriteStatusOpen);
    mp_int32 iRet = fileIO.OpenForWrite(bsaHandle, storgePath);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(ADDR(File, GetWriteStatus), GetWriteStatusClose);
    iRet = fileIO.OpenForWrite(bsaHandle, storgePath);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(ADDR(File, GetWriteStatus), GetWriteStatusClose);
    stub.set(ADDR(File, FileExist), StubFileExist);
    iRet = fileIO.OpenForWrite(bsaHandle, storgePath);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.reset(ADDR(File, FileExist));
    stub.set(ADDR(File, CreateFile), StubCreateFileFailed);
    iRet = fileIO.OpenForWrite(bsaHandle, storgePath);
    EXPECT_EQ(iRet, MP_FAILED);

    system("rm -f /tmp/OpenForWriteTest");
}

/*
* 测试用例：OpenForRead接口测试
* 前置条件：无
* CHECK点：1.读状态不为0时返回成功,2.读状态为0时文件不存在返回失败，文件存在返回成功.
*/
TEST_F(FileTest, OpenForReadTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    File fileIO;
    long bsaHandle = 0;
    std::string storgePath = "/tmp/OpenForReadTest";

    stub.set(ADDR(File, GetReadStatus), GetReadStatusOpen);
    mp_int32 iRet = fileIO.OpenForRead(bsaHandle, storgePath);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(ADDR(File, GetReadStatus), GetReadStatusClose);
    iRet = fileIO.OpenForRead(bsaHandle, storgePath);
    EXPECT_EQ(iRet, MP_FAILED);

    system("touch /tmp/OpenForReadTest");
    iRet = fileIO.OpenForRead(bsaHandle, storgePath);
    EXPECT_EQ(iRet, MP_SUCCESS);

    system("rm -f /tmp/OpenForReadTest");

    stub.set(ADDR(File, FileExist), StubFileExist);
    iRet = fileIO.OpenForRead(bsaHandle, storgePath);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(FileTest, FileExistTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    File fileIO;
    mp_string pszFilePath = "Agent";

    stub.set(stat, statFailed);
    mp_int32 iRet = fileIO.FileExist(pszFilePath);
    EXPECT_EQ(iRet, (int)MP_FALSE);

    stub.set(stat, statSuccess);
    iRet = fileIO.FileExist(pszFilePath);
    EXPECT_EQ(iRet, (int)MP_TRUE);

    stub.set(stat, statSuccessIsDir);
    iRet = fileIO.FileExist(pszFilePath);
    EXPECT_EQ(iRet, (int)MP_FALSE);
}

TEST_F(FileTest, CreateFileTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    File fileIO;
    mp_string pszFilePath;

    mp_int32 iRet = fileIO.CreateFile(pszFilePath);
    EXPECT_EQ(iRet, MP_FAILED);

    pszFilePath = "DWS";
    stub.set(ADDR(File, FileExist), FileExistSuccess);
    iRet = fileIO.CreateFile(pszFilePath);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(File, FileExist), FileExistFailed);
    stub.set(ADDR(File, ExecSystemCmd), ExecSystemCmdFailed);
    iRet = fileIO.CreateFile(pszFilePath);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(File, ExecSystemCmd), ExecSystemCmdSuccess);
    stub.set(fopen, fopenNull);
    iRet = fileIO.CreateFile(pszFilePath);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(FileTest, ExecSystemCmdTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    File fileIO;
    std::string strCommand;
    std::vector<std::string> strEcho;
    strCommand = "test";
    strEcho.push_back("test");

    stub.set(popen, popenNull);
    mp_int32 iRet = fileIO.ExecSystemCmd(strCommand, strEcho);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.reset(popen);
    // stub.set(feof, Stubpopen);
    iRet = fileIO.ExecSystemCmd(strCommand, strEcho);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(FileTest, ConnectArchiveTest)
{
    Stub stub;
    long bsaHandle;
    BsaHandleContext context;
    File fileIO;
    context.workingObj.archiveOpenSSL = 0;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);

    stub.set(ADDR(ArchiveStreamService, Connect), StubFailed);
    mp_int32 iRet = fileIO.ConnectArchive(bsaHandle, context);
    EXPECT_EQ(iRet, MP_FAILED);

    context.workingObj.archiveOpenSSL = 1;
    stub.set(ADDR(ArchiveStreamService, Connect), StubSuccess);
    iRet = fileIO.ConnectArchive(bsaHandle, context);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(FileTest, ReadFromArchiveTest)
{
    Stub stub;
    long bsaHandle;
    BSA_DataBlock32 dataBlockPtr;
    BsaHandleContext context;
    File fileIO;
    context.workingObj.archiveOpenSSL = 0;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);

    dataBlockPtr.bufferLen = 1;
    flag = 0;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32), StubSuccess);
    stub.set(ADDR(ArchiveStreamService, GetFileData), StubSuccessOnTwo);
    mp_int32 iRet = fileIO.ReadFromArchive(bsaHandle, &dataBlockPtr, context);
    EXPECT_EQ(iRet, MP_SUCCESS);

    flag = 0;
    dataBlockPtr.bufferLen = 1;
    context.workingObj.stFileInfo.readEnd = 1;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32), StubSuccess);
    stub.set(ADDR(ArchiveStreamService, GetFileData), StubSuccessOnTwo);
    iRet = fileIO.ReadFromArchive(bsaHandle, &dataBlockPtr, context);
    EXPECT_EQ(iRet, MP_SUCCESS);

    flag = 0;
    dataBlockPtr.bufferLen = 0;
    context.workingObj.stFileInfo.readEnd = 0;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32), StubSuccess);
    stub.set(ADDR(ArchiveStreamService, GetFileData), StubSuccessOnTwo);
    iRet = fileIO.ReadFromArchive(bsaHandle, &dataBlockPtr, context);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(FileTest, ReadTest)
{
    Stub stub;
    long bsaHandle;
    BSA_DataBlock32 dataBlockPtr;
    File fileIO;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);

    stub.set(fread, StubFreadOrFwrite);
    mp_int32 iRet = fileIO.Read(bsaHandle, &dataBlockPtr);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(FileTest, WriteTest)
{
    Stub stub;
    long bsaHandle;
    BSA_DataBlock32 dataBlockPtr;
    File fileIO;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);

    dataBlockPtr.numBytes = 0;
    stub.set(fwrite, StubFreadOrFwrite);
    mp_int32 iRet = fileIO.Write(bsaHandle, &dataBlockPtr);
    EXPECT_EQ(iRet, MP_SUCCESS);

    dataBlockPtr.numBytes = 1;
    iRet = fileIO.Write(bsaHandle, &dataBlockPtr);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(FileTest, GetLastTimeTest)
{
    Stub stub;
    File fileIO;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);

    fileIO.GetLastTime();
    fileIO.GetNowTime();
}