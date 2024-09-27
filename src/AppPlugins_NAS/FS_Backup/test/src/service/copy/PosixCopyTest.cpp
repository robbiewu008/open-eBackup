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
#include "stub.h"
#include "gtest/gtest.h"
#include "Backup.h"
#include "BackupMgr.h"
#include "ParserStructs.h"
#include "CopyCtrlParser.h"
#include "MetaParser.h"
#include "DirCacheParser.h"
#include "FileCacheParser.h"
#include "XMetaParser.h"
#include <boost/filesystem.hpp>

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    constexpr auto CONTROL_FILE = "/home/shuai/test_backup/ctrl/copy/control_0_b8bc6369eda84591ad40ebb42743ae19.txt";
    constexpr auto META_FILE_PATH = "/home/shuai/test_backup/meta/copy/latest";
    constexpr auto SRC_ROOT_PATH = "/";
    constexpr auto DST_ROOT_PATH = "/home/shuai/test_backup/dst";
    constexpr auto MODULE = "BACKUP_DEMO";
    const uint64_t KILOBYTE = 1024;
    const uint64_t MEGABYTE = 1024 * 1024;
    const uint64_t GIGABYTE = 1024 * 1024 * 1024;
    const uint64_t TARABYTE = 0x010000000000;
}

class PosixCopyBackupTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    void FillBackupParams(BackupParams& backupParams);
    void PrepareCtrlAndDcache();
    void FillDCacheFileParams(DirCacheParser::Params& dcacheParams);
    void FillFcacheFileParams(FileCacheParser::Params& fcacheParams);
    void FillDirCache(DirCache& dCache);
    void FillFcache(FileCache& fcache);
    void FillMetaFileParams(MetaParser::Params& metaParams);
    void FillDirMeta(DirMeta& dirMeta);
    void FillFileMeta(FileMeta& fileMeta);
    void FillXMetaFileParams(XMetaParser::Params& xmetaParams);
    void FillCopyCtrlParserParams(CopyCtrlParser::Params& params);
};

string GetParentDirName(const string path)
{
    return path.substr(0, path.find_last_of("/"));
}

void RecurseCreateDirectory(const string path)
{
    if (path.empty() || path[0] != '/') {
        return;
    }
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        return;
    }
    RecurseCreateDirectory(GetParentDirName(path));
    mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
}

void PosixCopyBackupTest::SetUp()
{

}

void PosixCopyBackupTest::TearDown()
{

}

void PosixCopyBackupTest::SetUpTestCase()
{

}

void PosixCopyBackupTest::TearDownTestCase()
{

}

void PosixCopyBackupTest::PrepareCtrlAndDcache()
{
    // first clear
    boost::filesystem::path tmpPath("/home/shuai/test_backup");
    boost::system::error_code ec;
    boost::filesystem::remove_all(tmpPath, ec);

    // mkdir /home/shuai/test_backup/ctrl/copy
    RecurseCreateDirectory("/home/shuai/test_backup/ctrl/copy");
    // mkdir /home/shuai/test_backup/meta/copy
    RecurseCreateDirectory("/home/shuai/test_backup/meta/copy/latest");
    // write dcache
    DirCacheParser::Params dcacheParams {};
    FillDCacheFileParams(dcacheParams);
    unique_ptr<DirCacheParser> dirCacheObj = make_unique<DirCacheParser>(dcacheParams);

    dirCacheObj->Open(CTRL_FILE_OPEN_MODE::WRITE);
    DirCache dirCache {};
    FillDirCache(dirCache);
    dirCacheObj->WriteDirCache(dirCache, DCACHE_WRITE_INFO::DCACHE_WRITE_TO_BUFFER);
    dirCacheObj->Close(CTRL_FILE_OPEN_MODE::WRITE);

    // write fcache
    FileCacheParser::Params fcacheParams {};
    FillFcacheFileParams(fcacheParams);
    unique_ptr<FileCacheParser> fcacheObj = make_unique<FileCacheParser>(fcacheParams);

    fcacheObj->Open(CTRL_FILE_OPEN_MODE::WRITE);
    FileCache fcache {};
    FillFcache(fcache);
    fcacheObj->WriteFileCache(fcache);
    fcacheObj->Close(CTRL_FILE_OPEN_MODE::WRITE);

    // write metafile
    MetaParser::Params metaParms {};
    FillMetaFileParams(metaParms);
    unique_ptr<MetaParser> metaObj = make_unique<MetaParser>(metaParms);
    metaObj->Open(CTRL_FILE_OPEN_MODE::WRITE);
    DirMeta dirMeta {};
    FillDirMeta(dirMeta);
    metaObj->WriteDirectoryMeta(dirMeta);
    FileMeta fileMeta {};
    FillFileMeta(fileMeta);
    metaObj->WriteFileMeta(fileMeta);
    metaObj->Close(CTRL_FILE_OPEN_MODE::WRITE);

    // write xmeta
    XMetaParser::Params xmetaParams {};
    FillXMetaFileParams(xmetaParams);
    unique_ptr<XMetaParser> xmetaObj = make_unique<XMetaParser>(xmetaParams);
    xmetaObj->Open(CTRL_FILE_OPEN_MODE::WRITE);
    
    XMetaField f1;
    f1.m_xMetaType = XMETA_TYPE::XMETA_TYPE_NAME;
    f1.m_value = "/home/shuai/test_backup/src";
    XMetaField f2;
    f2.m_xMetaType = XMETA_TYPE::XMETA_TYPE_ACL;
    f2.m_value = "default:&access:user::rwx group::--- other::--- ";
    vector<XMetaField> entry;
    entry.push_back(f1);
    entry.push_back(f2);
    xmetaObj->WriteXMeta(entry);
    xmetaObj->Close(CTRL_FILE_OPEN_MODE::WRITE);

    // write ctrlFile
    CopyCtrlParser::Params params {};
    FillCopyCtrlParserParams(params);
    unique_ptr<CopyCtrlParser> copyCtrlObj = make_unique<CopyCtrlParser>(params);
    copyCtrlObj->Open(CTRL_FILE_OPEN_MODE::WRITE);
    CopyCtrlDirEntry dirEntry;
    dirEntry.m_mode = "bm";
    dirEntry.m_dirName = "/home/shuai/test_backup/src";
    dirEntry.m_metaFileName = "meta_file_0";
    dirEntry.metaFileReadLen = 88;
    dirEntry.m_aclFlag = 1;
    dirEntry.metaFileOffset = 199;
    dirEntry.m_fileCount = 0;
    dirEntry.m_metaFileIndex = 0;
    copyCtrlObj->WriteDirEntry(dirEntry);
    CopyCtrlFileEntry fileEntry;
    fileEntry.m_mode = "dm";
    fileEntry.m_fileName = "test_backup.txt";
    fileEntry.m_metaFileName = "";
    fileEntry.metaFileReadLen = 120;
    fileEntry.m_aclFlag = 0;
    fileEntry.metaFileOffset = 287;
    fileEntry.m_fileSize = 12;
    fileEntry.m_metaFileIndex = 0;
    copyCtrlObj->WriteFileEntry(fileEntry);
    copyCtrlObj->Close(CTRL_FILE_OPEN_MODE::WRITE);
}

void PosixCopyBackupTest::FillDCacheFileParams(DirCacheParser::Params& params)
{
    params.fileName = "/home/shuai/test_backup/meta/copy/latest/dircache";
    params.maxEntriesPerFile = 100000;
    params.maxDataSize = 2147483648;
    params.taskId = "30016470";
    params.backupType = "FULL";
    params.nasServer = "";
    params.nasSharePath = "";
    params.proto = "FULL";
    params.protoVersion = "";
    params.metaDataScope = "folder-and-files";
}

void PosixCopyBackupTest::FillDirCache(DirCache& dcache)
{
    dcache.m_inode = 4200273;
    dcache.m_mdataOffset = 199;
    dcache.m_fcacheOffset = 195;
    dcache.m_dirPathHash.crc = 2349899083;
    dcache.m_dirMetaHash.crc = 2144100609;
    dcache.m_totalFiles = 1;
    dcache.m_fileId = 0;
    dcache.m_metaLength = 88;
    dcache.m_fcacheFileId = 0;
}

void PosixCopyBackupTest::FillFcache(FileCache& fcache)
{
    fcache.m_inode = 4198836;
    fcache.m_mdataOffset = 287;
    fcache.m_filePathHash.crc = 350224421;
    fcache.m_fileMetaHash.crc = 3464323256;
    fcache.m_fileId = 0;
    fcache.m_metaLength = 120;
    fcache.m_compareFlag = 2;
}

void PosixCopyBackupTest::FillFcacheFileParams(FileCacheParser::Params& params)
{
    params.fileName = "/home/shuai/test_backup/meta/copy/latest/filecache_0";
    params.readBufferSize = 32000;
    params.taskId = "30016470";
    params.backupType = "FULL";
    params.nasServer = "";
    params.nasSharePath = "";
    params.proto = "FULL";
    params.protoVersion = "";
    params.metaDataScope = "folder-and-files";
}

void PosixCopyBackupTest::FillMetaFileParams(MetaParser::Params& params)
{
    params.m_fileName = "/home/shuai/test_backup/meta/copy/latest/meta_file_0";
    params.maxEntriesPerFile = 100000;
    params.maxDataSize = 2147483648;
}

void PosixCopyBackupTest::FillDirMeta(DirMeta& dMeta)
{
    dMeta.type = 0;
    dMeta.m_attr = 0;
    dMeta.m_mode = 16832;
    dMeta.m_uid = 0;
    dMeta.m_gid = 0;
    dMeta.m_inode = 4200273;
    dMeta.m_size = 4096;
    dMeta.m_mtime = 1661245172;
    dMeta.m_ctime = 1661245196;
    dMeta.m_atime = 1661245172;
    dMeta.m_btime = 0;
    dMeta.m_xMetaFileIndex = 0;
    dMeta.m_xMetaFileOffset = 200;
}

void PosixCopyBackupTest::FillFileMeta(FileMeta& fMeta)
{
    fMeta.type = 0;
    fMeta.m_attr = 0;
    fMeta.m_mode = 33152;
    fMeta.m_nlink = 1;
    fMeta.m_uid = 0;
    fMeta.m_gid = 0;
    fMeta.m_inode = 4198836;
    fMeta.m_size = 12;
    fMeta.m_err = 0;
    fMeta.m_rdev = 0;
    fMeta.m_mtime = 1661245172;
    fMeta.m_ctime = 1661245172;
    fMeta.m_atime = 1661245172;
    fMeta.m_btime = 0;
    fMeta.m_blksize = 0;
    fMeta.m_blocks = 0;
    fMeta.m_xMetaFileIndex = 0;
    fMeta.m_xMetaFileOffset = 289;
}

void PosixCopyBackupTest::FillXMetaFileParams(XMetaParser::Params& params)
{
    params.m_fileName = "/home/shuai/test_backup/meta/copy/latest/xmeta_file_0";
    params.maxEntriesPerFile = 100000;
    params.maxDataSize = 2147483648;
}

void PosixCopyBackupTest::FillCopyCtrlParserParams(CopyCtrlParser::Params& params)
{
    params.maxEntriesPerFile = 100000;
    params.minEntriesPerFile = 10000;
    params.m_ctrlFileTimeElapsed = 5;
    params.maxDataSize = 2147483648;
    params.maxDataSize = 1073741824;
    params.m_ctlFileName = "/home/shuai/test_backup/ctrl/copy/control_0_b8bc6369eda84591ad40ebb42743ae19.txt";
    params.taskId = "30016470";
    params.backupType = "FULL";
    params.nasServer = "";
    params.nasSharePath = "";
    params.proto = "FULL";
    params.protoVersion = "";
    params.metaDataScope = "folder-and-files";
}

void PosixCopyBackupTest::FillBackupParams(BackupParams& backupParams)
{
    backupParams.srcEngine = BackupIOEngine::POSIX;
    backupParams.dstEngine = BackupIOEngine::POSIX;

    HostBackupAdvanceParams posixBackupAdvanceParams {};
    backupParams.srcAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    backupParams.dstAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->dataPath = SRC_ROOT_PATH;
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->dataPath = DST_ROOT_PATH;

    CommonParams commonParams {};
    commonParams.maxBufferCnt = 10;
    commonParams.maxBufferSize = 10 * 1024; // 10kb
    commonParams.maxErrorFiles = 100;
    commonParams.backupDataFormat = BackupDataFormat::NATIVE;
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    commonParams.jobId = "jobId";
    commonParams.subJobId = "subJobId";
    commonParams.skipFailure = true; // stop backup if any item backup failed.
    backupParams.commonParams = commonParams;

    ScanAdvanceParams scanAdvParams {};
    scanAdvParams.metaFilePath = META_FILE_PATH;
    backupParams.scanAdvParams = scanAdvParams;

    backupParams.backupType = BackupType::BACKUP_FULL;
    backupParams.phase = BackupPhase::COPY_STAGE;
}

/*
 * 用例名称:检查Posix备份流程
 * 前置条件：生成扫描结果文件controlFile 和 metaFile
 * check点：Posix备份流程顺利运行
 */
// TEST_F(PosixCopyBackupTest, testBackup)
// {
//     PrepareCtrlAndDcache();

//     unique_ptr<Backup> m_backup = nullptr;
//     BackupParams params {};
//     FillBackupParams(params);
//     m_backup = BackupMgr::CreateBackupInst(params);

//     if (BackupRetCode::SUCCESS != m_backup->Enqueue(CONTROL_FILE)) {
//         cout << "enqueue control file failed" << endl;
//     }

//     if (BackupRetCode::SUCCESS != m_backup->Start()) {
//         cout << "Start Backup failed" << endl;
//     }

//     BackupPhaseStatus backupStatus;
//     BackupStats stats;
//     do {
//         backupStatus = m_backup->GetStatus();
//         stats = m_backup->GetStats();

//         if (backupStatus == BackupPhaseStatus::COMPLETED) {
//             cout << "backup complete" << endl;
//             break;
//         } else if (backupStatus == BackupPhaseStatus::FAILED) {
//             cout << "backup failed" << endl;
//             break;
//         } else if (backupStatus == BackupPhaseStatus::INPROGRESS) {
//         }
//         sleep(1);
//     } while (true);

//     m_backup.reset();

//     EXPECT_EQ(backupStatus, BackupPhaseStatus::COMPLETED);
//     EXPECT_EQ(true, true);
// }