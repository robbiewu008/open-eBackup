/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: w30029850
 * Create: 2022/08/25
 */
#ifdef NAS_SNAPDIFF
#ifndef FS_SCANNER_NAS_SNAPDIFF_SERVICE_H
#define FS_SCANNER_NAS_SNAPDIFF_SERVICE_H

#include <memory>
#include <map>
#include <list>
#include <thread>
#include "NfsContextWrapper.h"
#include "SmbContextWrapper.h"
#include "SnapdiffBackupEntry.h"
#include "ParserUtils.h"
#include "ControlDevice.h"
#include "CommonService.h"
#include "BufferQueue.h"

class SnapdiffService {
public:
    explicit SnapdiffService(ScanConfig &config,
        std::shared_ptr<BufferQueue<SnapdiffResultMap>> &snapdiffBufferQueue,
        std::shared_ptr<StatisticsMgr> &statsMgr);
    ~SnapdiffService();

    bool InitControlFilesAndMetaFile();
    SCANNER_STATUS Start();

private:
    void ValidateConfigForSnapdiffService(ScanConfig &config);
    void MainLoop();

    void CleanMetaControlFile();

    bool InitMetaFile();
    bool InitXMetaFile();
    bool InitCopyCtrlFile();
    bool InitHardLinkCtrlFile();
    bool InitMtimeCtrlFile();
    bool InitDeleteCtrlFile();

    bool InitNasSession();
    bool InitSmbSession();
    void CloseSmbSession();
    bool InitNFSSession();
    void CloseNFSSession();
    
    SNAPDIFF_BACKUPENTRY_CHANGETYPE FetchDirChangeType(const std::string &dirPath);
    void WriteFromDiffResultMap(const std::string &dirPath, std::list<Module::SnapdiffMetadataInfo>& diffInfoList);
    void WriteDirDiffMetaControl(SnapdiffBackupEntry &backupEntry,
        bool &isDirInsertDeleteCtrl, Module::SnapdiffMetadataInfo& diffInfo);
    void WriteFileDiffMetaControl(SnapdiffBackupEntry &backupEntry, bool &isDirInsertDeleteCtrl,
        std::list<Module::SnapdiffMetadataInfo>& diffInfoList);
    void CutFilenameSuffix(std::string &filePath, const std::string &suffix);

    SnapdiffBackupEntry DiffInputToBackup(const Module::SnapdiffMetadataInfo &snapdiffMetaDataInfo);
    bool FillDiffLibNfs(SnapdiffBackupEntry &backupEntry);
    bool FillDiffLibSmb(SnapdiffBackupEntry &backupEntry);
    bool IsRetryMount(int errorCode);
    std::string GetSmbAcl(const std::string &path);

    void WriteDirToMetaFile(const SnapdiffBackupEntry &backupEntry);
    void WriteFileToMetaFile(const SnapdiffBackupEntry &backupEntry);
    void WriteDirToControlFile(const SnapdiffBackupEntry &backupEntry, uint64_t preMetaFileOffset);
    void WriteFileToControlFile(const SnapdiffBackupEntry &backupEntry, uint64_t preMetaFileOffset);
    void WriteToMtimeControlFile(const SnapdiffBackupEntry &backupEntry);
    void WriteToDeleteControlFile(const std::string &path, bool isDir, bool isDelete);

    void PostMetaFileWrite(uint64_t offset);
    void PostXMetaFileWrite(uint64_t offset);
    void PostCopyCtrlFileWrite(Module::CTRL_FILE_RETCODE retCode);
    void PostMtimeCtrlFileWrite(Module::CTRL_FILE_RETCODE retCode);
    void PostDeleteCtrlFileWrite(Module::CTRL_FILE_RETCODE retCode);
    void PostHardlinkFileWrite(Module::CTRL_FILE_RETCODE retCode);

private:
    ScanConfig &m_config;
    std::shared_ptr<BufferQueue<SnapdiffResultMap>> m_buffer;
    std::shared_ptr<StatisticsMgr> m_statsMgr;
    std::shared_ptr<std::thread> m_mainLoopThread;

    std::string m_directory {};
    std::string m_ctrlFileDirectory {};

    std::shared_ptr<Module::NfsContextWrapper> m_nfsCtx;
    std::shared_ptr<Module::SmbContextWrapper> m_smbCtx;

    uint32_t m_metaFileCount = 0;
    uint32_t m_XmetaFileCount = 0;
    uint32_t m_copyCtrlFileCount = 0;
    uint32_t m_mtimeCtrlFileCount = 0;
    uint32_t m_deleteCtrlFileCount = 0;
    uint32_t m_hardlinkCtrlFileCount = 0;

    std::shared_ptr<Module::MetaParser> m_MetaParser;
    std::shared_ptr<Module::XMetaParser> m_XMetaParser;
    std::shared_ptr<Module::CopyCtrlParser> m_copyCtrlParser;
    std::shared_ptr<Module::MtimeCtrlParser> m_mtimeCtrlParser;
    std::shared_ptr<Module::DeleteCtrlParser> m_deleteCtrlParser;
    std::shared_ptr<Module::HardlinkCtrlParser> m_hardlinkCtrlParser;

    std::string m_metaFilePath {};
    std::string m_XmetaFilePath {};
    std::string m_copyCtrlFilePath {};
    std::string m_mtimeCtrlFilePath {};
    std::string m_deleteCtrlFilePath {};
    std::string m_hardlinkCtrlFilePath {};

    std::map<std::string, SNAPDIFF_BACKUPENTRY_CHANGETYPE> m_tmpDirChangeType {};
};

#endif
#endif