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
#include <iostream>
#include <list>
#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <common/Macros.h>
#include <repository_handlers/RepositoryHandler.h>
#include <repository_handlers/filesystem/FileSystemHandler.h>
#include <securec.h>
#include <log/Log.h>

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace VirtPlugin;

namespace HDT_TEST {
const std::string g_fileContent = "FILE CONTENT IN THE TEST FILE.";
const std::string g_fileToOpen = "/tmp/fs_test_files/FileSystemHandlerTest_file2open.txt";

void PrepareFile4Test();
void CleanFile4Test();

class FileSystemHandlerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    void InitLogger()
    {
        std::string logFileName = "file_system_handler_test.log";
        std::string logFilePath = "/tmp/log/";
        int logLevel = DEBUG;
        int logFileCount = 10;
        int logFileSize = 30;
        Module::CLogger::GetInstance().Init(
            logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
    }
public:
    std::shared_ptr<RepositoryHandler> m_fsHandler;
};

void FileSystemHandlerTest::SetUp()
{
    InitLogger();
    PrepareFile4Test();
    m_fsHandler = std::make_shared<FileSystemHandler>();
}

void FileSystemHandlerTest::TearDown()
{
    CleanFile4Test();
}

void FileSystemHandlerTest::SetUpTestCase() {}
void FileSystemHandlerTest::TearDownTestCase() {}

/*
 * 测试用例： 打开和关闭文件
 * 前置条件： 文件存在
 * CHECK点： 打开文件成功；关闭文件成功
 */
TEST_F(FileSystemHandlerTest, OpenAndClose)
{
    INFOLOG("OpenAndClose");
    std::string openMode[] = { "r", "r+", "w", "w+", "a", "a+" };
    for (auto mode : openMode) {
        EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, mode), SUCCESS);
        EXPECT_EQ(m_fsHandler->Close(), SUCCESS);
    }
}

/*
 * 测试用例： 打开文件失败
 * 前置条件： 文件不存在
 * CHECK点： 打开文件返回非SUCCESS
 */
TEST_F(FileSystemHandlerTest, Open_Failed)
{
    INFOLOG("Open_Failed");
    EXPECT_NE(m_fsHandler->Open("/tmp/fs_test_files/not_exist_file.txt", "r"), FAILED);
}

/*
 * 测试用例： 从文件读数据
 * 前置条件： 打开文件成功
 * CHECK点： 读取数据长度符合预期；读取到的数据内容符合预期；
 */
TEST_F(FileSystemHandlerTest, Read)
{
    INFOLOG("Read");
    EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, "r"), SUCCESS);

    const size_t sizeToRead = 5;
    std::shared_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(sizeToRead);

    size_t readSize = m_fsHandler->Read(buf, sizeToRead);
    EXPECT_EQ(readSize, sizeToRead);
    EXPECT_EQ(m_fsHandler->Close(), SUCCESS);

    int cmpRet = memcmp(buf.get(), g_fileContent.c_str(), sizeToRead);
    EXPECT_EQ(cmpRet, 0);
}

/*
 * 测试用例： 从文件读数据失败
 * 前置条件： 未打开文件
 * CHECK点： Read返回读取失败
 */
TEST_F(FileSystemHandlerTest, ReadFailed)
{
    INFOLOG("ReadFailed");
    std::string content;
    const size_t sizeToRead = 5;
    std::shared_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(sizeToRead);
    EXPECT_EQ(m_fsHandler->Read(buf, sizeToRead), 0);
    EXPECT_EQ(m_fsHandler->Read(content, sizeToRead), 0);
}

/*
 * 测试用例： 从文件读数据到string
 * 前置条件： 打开文件成功
 * CHECK点： 读取数据长度符合预期；读取到的数据内容符合预期；
 */
TEST_F(FileSystemHandlerTest, Read2String)
{
    INFOLOG("Read2String");
    EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, "r"), SUCCESS);

    std::string content;
    size_t fileSize = m_fsHandler->FileSize(g_fileContent);
    size_t readSize = m_fsHandler->Read(content, fileSize);
    EXPECT_EQ(readSize, fileSize);
    EXPECT_EQ(m_fsHandler->Close(), SUCCESS);

    int cmpRet = memcmp(content.c_str(), g_fileContent.c_str(), fileSize);
    EXPECT_EQ(cmpRet, 0);
}

/*
 * 测试用例： 写数据
 * 前置条件： 打开文件成功
 * CHECK点： 写入长度符合预期；写入后flush成功；写入后关闭文件成功；
 */
TEST_F(FileSystemHandlerTest, WriteAndFlush)
{
    INFOLOG("WriteAndFlush");
    EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, "w"), SUCCESS);

    const size_t sizeToWrite = 10;
    std::shared_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(sizeToWrite);
    const char * contentToWrite = "0123456789";
    memcpy_s(buf.get(), sizeToWrite, contentToWrite, strlen(contentToWrite));
    size_t wSize = m_fsHandler->Write(buf, sizeToWrite);

    EXPECT_EQ(wSize, sizeToWrite);
    EXPECT_EQ(m_fsHandler->Flush(false), true);
    EXPECT_EQ(m_fsHandler->Close(), SUCCESS);
}

/*
 * 测试用例： 附加写
 * 前置条件： 打开文件成功
 * CHECK点： 写入长度符合预期；写入后flush成功；写入后关闭文件成功；
 */
TEST_F(FileSystemHandlerTest, AppendAndFlush)
{
    INFOLOG("AppendAndFlush");
    EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, "a"), SUCCESS);

    const size_t sizeToWrite = 10;
    std::shared_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(sizeToWrite);
    const char * contentToWrite = "0123456789";
    memcpy_s(buf.get(), sizeToWrite, contentToWrite, strlen(contentToWrite));
    size_t wSize = m_fsHandler->Write(buf, sizeToWrite);

    EXPECT_EQ(wSize, sizeToWrite);
    EXPECT_EQ(m_fsHandler->Flush(true), true);
    EXPECT_EQ(m_fsHandler->Close(), SUCCESS);
}

/*
 * 测试用例： 获取当前文件偏移
 * 前置条件： 打开文件成功
 * CHECK点： 返回偏移符合预期
 */
TEST_F(FileSystemHandlerTest, Tell)
{
    INFOLOG("Tell");
    EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, "r"), SUCCESS);

    const size_t sizeToRead = 5;
    std::shared_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(sizeToRead);
    size_t readSize = m_fsHandler->Read(buf, sizeToRead);
    EXPECT_EQ(readSize, sizeToRead);

    size_t tellRet = m_fsHandler->Tell();
    EXPECT_EQ(tellRet, sizeToRead);
    EXPECT_EQ(m_fsHandler->Close(), SUCCESS);
}

/*
 * 测试用例： 设置当前文件偏移
 * 前置条件： 打开文件成功
 * CHECK点： 写入内容偏移量符合预期
 */
TEST_F(FileSystemHandlerTest, Seek)
{
    INFOLOG("Seek");
    EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, "r"), SUCCESS);

    const size_t sizeToRead = 10;
    std::shared_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(sizeToRead);
    size_t readSize = m_fsHandler->Read(buf, sizeToRead);
    EXPECT_EQ(readSize, sizeToRead);
    size_t tellRet = m_fsHandler->Tell();
    EXPECT_EQ(tellRet, sizeToRead);

    const size_t offset = 5;
    size_t seekRet = m_fsHandler->Seek(offset);
    EXPECT_EQ(seekRet, SUCCESS);

    tellRet = m_fsHandler->Tell();
    EXPECT_EQ(tellRet, offset);
    EXPECT_EQ(m_fsHandler->Close(), SUCCESS);
}

/*
 * 测试用例： 获取文件大小
 * 前置条件： 文件存在
 * CHECK点： 返回文件长度符合预期
 */
TEST_F(FileSystemHandlerTest, FileSize)
{
    INFOLOG("FileSize");
    size_t fileSize = m_fsHandler->FileSize(g_fileToOpen);
    EXPECT_EQ(fileSize, strlen(g_fileContent.c_str()));
    EXPECT_EQ(m_fsHandler->Close(), SUCCESS);
}

/*
 * 测试用例： 判断文件是否存在
 * 前置条件： 文件存在
 * CHECK点： 返回成功
 */
TEST_F(FileSystemHandlerTest, Exists)
{
    INFOLOG("Exists");
    EXPECT_EQ(m_fsHandler->Exists(g_fileToOpen), true);
}

/*
 * 测试用例： 判断文件是否存在
 * 前置条件： 文件不存在
 * CHECK点： 返回失败
 */
TEST_F(FileSystemHandlerTest, Exists_NotExists)
{
    INFOLOG("Exists_NotExists");
    EXPECT_EQ(m_fsHandler->Exists("/tmp/fs_test_files/not_exist_file.txt"), false);
}

/*
 * 测试用例： 重命名文件
 * 前置条件： 文件存在
 * CHECK点： 返回成功
 */
TEST_F(FileSystemHandlerTest, Rename)
{
    INFOLOG("Rename");
    EXPECT_EQ(m_fsHandler->Rename(g_fileToOpen, "/tmp/fs_test_files/rename_case.txt"), true);
}

/*
 * 测试用例： 重命名文件
 * 前置条件： 文件不存在；输入空字符串；
 * CHECK点： 返回失败
 */
TEST_F(FileSystemHandlerTest, Rename_Failed)
{
    INFOLOG("Rename_Failed");
    EXPECT_EQ(m_fsHandler->Rename("", ""), false);
    EXPECT_EQ(m_fsHandler->Rename(" ", " "), false);
    EXPECT_EQ(m_fsHandler->Rename(
        "/tmp/fs_test_files/not_exist_file.txt", "/tmp/fs_test_files/rename_case.txt"), false);
}

/*
 * 测试用例： 判断是否为目录
 * 前置条件： 目录存在
 * CHECK点： 返回成功
 */
TEST_F(FileSystemHandlerTest, IsDirectory)
{
    INFOLOG("IsDirectory");
    EXPECT_EQ(m_fsHandler->IsDirectory("/tmp"), true);
}

/*
 * 测试用例： 判断是否为目录
 * 前置条件： 传入文件名
 * CHECK点： 返回失败
 */
TEST_F(FileSystemHandlerTest, IsDirectory_Failed)
{
    INFOLOG("IsDirectory_Failed");
    EXPECT_EQ(m_fsHandler->IsDirectory(g_fileToOpen), false);
}

/*
 * 测试用例： 判断是否为常规文件
 * 前置条件： 传入文件名
 * CHECK点： 返回成功
 */
TEST_F(FileSystemHandlerTest, IsRegularFile)
{
    INFOLOG("IsRegularFile");
    EXPECT_EQ(m_fsHandler->IsRegularFile(g_fileToOpen), true);
    EXPECT_EQ(m_fsHandler->IsRegularFile("/not/exists"), false);
}

/*
 * 测试用例： 判断是否为常规文件
 * 前置条件： 传入目录
 * CHECK点： 返回失败
 */
TEST_F(FileSystemHandlerTest, IsRegularFile_Failed)
{
    INFOLOG("IsRegularFile_Failed");
    EXPECT_EQ(m_fsHandler->IsRegularFile("/tmp"), false);
}

/*
 * 测试用例： 删除文件/目录
 * 前置条件： 如果是目录则目录非空
 * CHECK点： 删除成功
 */
TEST_F(FileSystemHandlerTest, Remove)
{
    INFOLOG("Remove");
    system("mkdir -p /tmp/fs_test_files/fs_test_for_delete/");
    system("touch /tmp/fs_test_files/fs_test_for_delete/file_to_delete.txt");

    EXPECT_EQ(m_fsHandler->Remove("/tmp/fs_test_files/fs_test_for_delete/file_to_delete.txt"), true);
    EXPECT_EQ(m_fsHandler->Remove("/tmp/fs_test_files/fs_test_for_delete"), true);
    EXPECT_EQ(m_fsHandler->Remove("/tmp/fs_test_files/fs_test_for_delete_notexist"), true);
}

/*
 * 测试用例： 删除文件/目录
 * 前置条件： 目录非空
 * CHECK点： 删除失败
 */
TEST_F(FileSystemHandlerTest, Remove_Failed)
{
    INFOLOG("Remove_Failed");
    system("mkdir -p /tmp/fs_test_files/fs_test_for_delete/");
    system("touch /tmp/fs_test_files/fs_test_for_delete/file_to_delete.txt");

    EXPECT_EQ(m_fsHandler->Remove("/tmp/fs_test_files/fs_test_for_delete/"), false);
    EXPECT_EQ(m_fsHandler->RemoveAll("/tmp/fs_test_files/fs_test_for_delete/"), true);
}

/*
 * 测试用例： 删除目录及其子目录
 * 前置条件： 目录存在
 * CHECK点： 删除成功
 */
TEST_F(FileSystemHandlerTest, RemoveAll)
{
    INFOLOG("RemoveAll");
    system("mkdir -p /tmp/fs_test_files/fs_test_for_delete/dir/empty_dir");
    system("mkdir -p /tmp/fs_test_files/fs_test_for_delete/dir2/dir3/dir4");
    system("mkdir -p /tmp/fs_test_files/fs_test_for_delete/");
    system("touch    /tmp/fs_test_files/fs_test_for_delete/file_to_delete.txt");

    EXPECT_EQ(m_fsHandler->RemoveAll("/tmp/fs_test_files/fs_test_for_delete/"), true);
    EXPECT_EQ(m_fsHandler->Exists("/tmp/fs_test_files/fs_test_for_delete/"), false);
    EXPECT_EQ(m_fsHandler->Exists("/tmp/fs_test_files/fs_test_for_delete/file_to_delete.txt"), false);
}

/*
 * 测试用例： 删除目录及其子目录
 * 前置条件： 目录不存在；传入文件；
 * CHECK点： 删除失败
 */
TEST_F(FileSystemHandlerTest, RemoveAll_Failed)
{
    INFOLOG("RemoveAll_Failed");
    system("mkdir -p /tmp/fs_test_files/fs_test_for_delete/");
    system("touch /tmp/fs_test_files/fs_test_for_delete/file_to_delete.txt");

    EXPECT_EQ(m_fsHandler->RemoveAll("/tmp/fs_test_files/fs_test_for_delete/file_to_delete.txt"), false);
    EXPECT_EQ(m_fsHandler->RemoveAll("/tmp/fs_test_files/fs_test_for_delete_notexist"), false);
    EXPECT_EQ(m_fsHandler->Exists("/tmp/fs_test_files/fs_test_for_delete/"), true);
    EXPECT_EQ(m_fsHandler->Exists("/tmp/fs_test_files/fs_test_for_delete/file_to_delete.txt"), true);

    system("rm -rf /tmp/fs_test_files/fs_test_for_delete/");
}

/*
 * 测试用例： 获取目录下的文件信息
 * 前置条件： 目录存在
 * CHECK点： 删除成功
 */
TEST_F(FileSystemHandlerTest, GetFiles)
{
INFOLOG("GetFiles");
system("mkdir -p /tmp/fs_test_files/fs_test_for_getfiles");
system("touch    /tmp/fs_test_files/fs_test_for_getfiles/file_to_delete.txt");

std::string pathName = "/tmp/fs_test_files/fs_test_for_getfiles";
std::vector<std::string> files {};
m_fsHandler->GetFiles(pathName,files);
auto singleFile = files.front();
std::cout<< singleFile << std::endl;
EXPECT_EQ(singleFile, "/tmp/fs_test_files/fs_test_for_getfiles/file_to_delete.txt");
system("rm -rf /tmp/fs_test_files/fs_test_for_getfiles/");
}


/*
 * 测试用例： 递归创建目录
 * 前置条件： 无
 * CHECK点： 创建成功；检查创建的目录存储在；
 */
TEST_F(FileSystemHandlerTest, CreateDirectory)
{
    INFOLOG("CreateDirectory");
    EXPECT_EQ(m_fsHandler->CreateDirectory(
        "////tmp/fs_test_files/fs_test_for_create_dir//////dir/dir/dir/dir/dir/dir"), true);
    EXPECT_EQ(m_fsHandler->CreateDirectory(
        "////tmp/fs_test_files/fs_test_for_create_dir////dir/dir/dir/dir/dir/dir"), true);
    EXPECT_EQ(m_fsHandler->Exists("/tmp/fs_test_files/fs_test_for_create_dir/dir/dir"), true);
    EXPECT_EQ(m_fsHandler->RemoveAll("/tmp/fs_test_files/fs_test_for_create_dir/"), true);
    EXPECT_EQ(m_fsHandler->CreateDirectory(""), false);
}

/*
 * 测试用例： 复制文件为另一个文件
 * 前置条件： 目标位置目录存在，文件无同名文件
 * CHECK点： 成功
 */
TEST_F(FileSystemHandlerTest, CopyFile_DestFileNotExist)
{
    INFOLOG("CopyFile_DestFileNotExist");
    std::string src = "/tmp/fs_test_files/src_dir/source.file";
    std::string dst = "/tmp/fs_test_files/dst_dir/dest.file";
    EXPECT_EQ(m_fsHandler->CreateDirectory("/tmp/fs_test_files/src_dir/"), true);
    EXPECT_EQ(m_fsHandler->CreateDirectory("/tmp/fs_test_files/dst_dir/"), true);

    std::string createSrcFile = "touch " + src;
    system(createSrcFile.c_str());

    EXPECT_EQ(m_fsHandler->Exists(src), true);
    EXPECT_EQ(m_fsHandler->Exists(dst), false);
    EXPECT_EQ(m_fsHandler->CopyFile(src, dst), true);
    EXPECT_EQ(m_fsHandler->Exists(dst), true);
    EXPECT_EQ(m_fsHandler->RemoveAll("/tmp/fs_test_files/"), true);
}

/*
 * 测试用例： 复制文件为另一个文件
 * 前置条件： 目标位置目录存在，有同名文件
 * CHECK点： 成功
 */
TEST_F(FileSystemHandlerTest, CopyFile_DestFileExist)
{
    INFOLOG("CopyFile_DestFileExist");
    std::string src = "/tmp/fs_test_files/src_dir/source.file";
    std::string dst = "/tmp/fs_test_files/dst_dir/dest.file";
    EXPECT_EQ(m_fsHandler->CreateDirectory("/tmp/fs_test_files/src_dir/"), true);
    EXPECT_EQ(m_fsHandler->CreateDirectory("/tmp/fs_test_files/dst_dir/"), true);

    std::string createSrcFile = "touch " + src;
    std::string createDstFile = "touch " + dst;
    system(createSrcFile.c_str());
    system(createDstFile.c_str());

    EXPECT_EQ(m_fsHandler->Exists(src), true);
    EXPECT_EQ(m_fsHandler->Exists(dst), true);
    EXPECT_EQ(m_fsHandler->CopyFile(src, dst), true);
    EXPECT_EQ(m_fsHandler->Exists(dst), true);
    EXPECT_EQ(m_fsHandler->RemoveAll("/tmp/fs_test_files/"), true);
}

/*
 * 测试用例： 复制文件到目录
 * 前置条件： 目标位置目录不存在
 * CHECK点： 失败
 */
TEST_F(FileSystemHandlerTest, CopyFile_DestDirNotExist)
{
    INFOLOG("CopyFile_DestDirNotExist");
    std::string src = "/tmp/fs_test_files/src_dir/source.file";
    std::string dst = "/tmp/fs_test_files/dst_dir/";
    EXPECT_EQ(m_fsHandler->CreateDirectory("/tmp/fs_test_files/src_dir/"), true);

    std::string createSrcFile = "touch " + src;
    system(createSrcFile.c_str());

    EXPECT_EQ(m_fsHandler->Exists(src), true);
    EXPECT_EQ(m_fsHandler->Exists(dst), false);
    EXPECT_EQ(m_fsHandler->CopyFile(src, dst), false);
    EXPECT_EQ(m_fsHandler->Exists(dst + "source.file"), false);
    EXPECT_EQ(m_fsHandler->RemoveAll("/tmp/fs_test_files/"), true);
}

/*
 * 测试用例： 复制文件到目标目录
 * 前置条件： 目标位置目录存在
 * CHECK点： 成功
 */
TEST_F(FileSystemHandlerTest, CopyFile_DestDirExist)
{
    INFOLOG("CopyFile_DestDirExist");
    std::string src = "/tmp/fs_test_files/src_dir/source.file";
    std::string dst = "/tmp/fs_test_files/dst_dir/";
    EXPECT_EQ(m_fsHandler->CreateDirectory("/tmp/fs_test_files/src_dir/"), true);
    EXPECT_EQ(m_fsHandler->CreateDirectory("/tmp/fs_test_files/dst_dir/"), true);

    std::string createSrcFile = "touch " + src;
    system(createSrcFile.c_str());

    EXPECT_EQ(m_fsHandler->Exists(src), true);
    EXPECT_EQ(m_fsHandler->Exists(dst), true);
    EXPECT_EQ(m_fsHandler->CopyFile(src, dst), true);
    EXPECT_EQ(m_fsHandler->Exists(dst + "source.file"), true);
    EXPECT_EQ(m_fsHandler->RemoveAll("/tmp/fs_test_files/"), true);
}

/*
 * 测试用例： 复制文件到目标目录
 * 前置条件： 源文件不存在
 * CHECK点： 失败
 */
TEST_F(FileSystemHandlerTest, CopyFile_SrcFileNotExist)
{
    INFOLOG("CopyFile_SrcFileNotExist");
    std::string src = "/tmp/fs_test_files/src_dir/source.file";
    std::string dst = "/tmp/fs_test_files/dst_dir/";
    EXPECT_EQ(m_fsHandler->CreateDirectory("/tmp/fs_test_files/src_dir/"), true);
    EXPECT_EQ(m_fsHandler->CreateDirectory("/tmp/fs_test_files/dst_dir/"), true);
    EXPECT_EQ(m_fsHandler->Exists(src), false);
    EXPECT_EQ(m_fsHandler->Exists(dst), true);
    EXPECT_EQ(m_fsHandler->CopyFile(src, dst), false);
    EXPECT_EQ(m_fsHandler->Exists(dst + "source.file"), false);
    EXPECT_EQ(m_fsHandler->RemoveAll("/tmp/fs_test_files/"), true);
}

/*
 * 测试用例： 复制目录到目标目录
 * 前置条件： 无
 * CHECK点： 失败
 */
TEST_F(FileSystemHandlerTest, CopyFile_CopyDirFailed)
{
    INFOLOG("CopyFile_CopyDirFailed");
    std::string src = "/tmp/fs_test_files/src_dir";
    std::string dst = "/tmp/fs_test_files/dst_dir";
    EXPECT_EQ(m_fsHandler->CreateDirectory(src), true);
    EXPECT_EQ(m_fsHandler->CreateDirectory(dst), true);
    EXPECT_EQ(m_fsHandler->Exists(src), true);
    EXPECT_EQ(m_fsHandler->Exists(dst), true);
    EXPECT_EQ(m_fsHandler->CopyFile(src, dst), false);
    EXPECT_EQ(m_fsHandler->Exists(dst + "/src_dir"), false);
    EXPECT_EQ(m_fsHandler->RemoveAll("/tmp/fs_test_files/"), true);
}

/*
 * 测试用例： 复制文件/目录失败
 * 前置条件： 源或者目的路径为空
 * CHECK点： 失败
 */
TEST_F(FileSystemHandlerTest, CopyFile_SrcORDestEmpty)
{
    INFOLOG("CopyFile_SrcORDestEmpty");
    std::string src = "";
    std::string dst = "/tmp/fs_test_files/dst_dir";
    EXPECT_EQ(m_fsHandler->CopyFile(src, dst), false);

    src = "/tmp/fs_test_files/src_dir";
    dst = "";
    EXPECT_EQ(m_fsHandler->CopyFile(src, dst), false);
}

void PrepareFile4Test()
{
    std::string cmd2CreateTestDir = "mkdir -p /tmp/fs_test_files/";
    system(cmd2CreateTestDir.c_str());

    std::string g_fileToOpen = "/tmp/fs_test_files/FileSystemHandlerTest_file2open.txt";
    std::string cmd2CreateFile = "echo -n " + g_fileContent + ">" + g_fileToOpen;
    system(cmd2CreateFile.c_str());
}

void CleanFile4Test()
{
    std::string cmd2RemoveTestDir = "rm -rf /tmp/fs_test_files/";
    system(cmd2RemoveTestDir.c_str());
}

/*
 * 测试用例： Truncate成功
 * 前置条件： NA
 * CHECK点： 打开文件返回非SUCCESS
 */
TEST_F(FileSystemHandlerTest, TruncateSuccess)
{
    INFOLOG("TruncateSuccess");
    size_t fileSize = 100;
    /* truncate success */
    EXPECT_EQ(m_fsHandler->Remove(g_fileToOpen), true);
    EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, "a+"), SUCCESS);
    EXPECT_EQ(m_fsHandler->Truncate(fileSize), SUCCESS);
    EXPECT_EQ(m_fsHandler->Close(), SUCCESS);
    EXPECT_EQ(m_fsHandler->FileSize(g_fileToOpen), fileSize);
}

/*
 * 测试用例： Truncate失败
 * 前置条件： NA
 * CHECK点： 打开文件返回非SUCCESS
 */
TEST_F(FileSystemHandlerTest, TruncateFailed)
{
    INFOLOG("TruncateFailed");
    size_t fileSize = 100;
    /* truncate failed since file not open */
    EXPECT_EQ(m_fsHandler->Truncate(fileSize), FAILED);
    /* truncate failed since open mode incorrect */
    EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, "r"), SUCCESS);
    EXPECT_NE(m_fsHandler->Truncate(fileSize), SUCCESS);
}

/*
 * 测试用例： Append成功
 * 前置条件： NA
 * CHECK点： 打开文件返回非SUCCESS
 */
TEST_F(FileSystemHandlerTest, AppendSuccess)
{
    INFOLOG("AppendSuccess");
    std::shared_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(50);
    memset(buf.get(), 'a', 50);
    EXPECT_EQ(m_fsHandler->Remove(g_fileToOpen), true);
    EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, "a+"), SUCCESS);
    EXPECT_EQ(m_fsHandler->Append(buf, 50), 50);
    EXPECT_EQ(m_fsHandler->Close(), SUCCESS);
    EXPECT_EQ(m_fsHandler->FileSize(g_fileToOpen), 50);
}

/*
 * 测试用例： Append失败
 * 前置条件： NA
 * CHECK点： 打开文件返回非SUCCESS
 */
TEST_F(FileSystemHandlerTest, AppendFailed)
{
    INFOLOG("AppendFailed");
    std::shared_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(50);
    /* append failed since file not open */
    EXPECT_EQ(m_fsHandler->Append(buf, 50), 0); /* 0 - failed */
    /* append failed since open mode incorrect */
    EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, "w+"), SUCCESS);
    EXPECT_EQ(m_fsHandler->Append(buf, 50), 0); /* 0 - failed */
    EXPECT_EQ(m_fsHandler->Close(), SUCCESS);

    /* append failed since file not exists */
    EXPECT_EQ(m_fsHandler->Open(g_fileToOpen, "w+"), SUCCESS);
    EXPECT_EQ(m_fsHandler->Remove(g_fileToOpen), true);
    EXPECT_EQ(m_fsHandler->Append(buf, 50), 0); /* 0 - failed */
}
}
