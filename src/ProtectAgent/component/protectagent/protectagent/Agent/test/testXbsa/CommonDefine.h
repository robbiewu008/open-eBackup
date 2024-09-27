#ifndef _COMMON_DEFINE_H_
#define _COMMON_DEFINE_H_
#include <iostream>
#include <time.h>
#include <memory>
#include <string.h>
#include <unistd.h>
#include <vector>

#define TEST_API
enum BUSINESS_TYPE
{
    Backup = 0,
    Recover = 1, 
    Delete = 2
};

const std::string TEST_BACKUP_DIR = "/home/rdadmin/test/backup/";
const std::string TEST_DELETE_DIR = "/home/rdadmin/test/delete/";
const std::string TEST_RECOVER_DIR = "/home/rdadmin/test/recover/";

#define Log(format, args...) \
{\
    time_t tt; \
    time( &tt ); \
    tm* t= gmtime( &tt ); \
    char timeStr[100]; \
    memset(timeStr, 0, 100); \
    sprintf(timeStr, "%d-%02d-%02d %02d:%02d:%02d", t->tm_year + 1900,t->tm_mon + 1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec); \
    int tempPid = getpid(); \
    std::cout << std::string(timeStr) << " [" << tempPid << "]  ";\
    printf(format, ##args);\
    std::cout << "\n"; \
}

TEST_API bool ExecSystemCmd(const std::string& strCommand, std::vector<std::string>& strEcho);

TEST_API bool CreateNewDir(const std::string &newDir);

TEST_API bool CopyFile(const std::string &srcFile, const std::string &dstFile);

TEST_API std::string GenerateSerial();
TEST_API bool DeleteFile(const std::string &fileName);
#endif