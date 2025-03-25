/*
* Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
* Description: Prouce meta&xmeta base log files for object storage.
* Author: w00444223
* Create: 2024-02-19
*/

#ifndef FS_SCANNER_OBJECT_LOG_PROCESS_H
#define FS_SCANNER_OBJECT_LOG_PROCESS_H

#include "ObjectMetaReadThread.h"

class ObjectLogProcess {
public:
    explicit ObjectLogProcess(ScanConfig& config,
        std::shared_ptr<BufferQueue<DirectoryScan>> output, std::shared_ptr<StatisticsMgr> statsMgr);
    ~ObjectLogProcess() {};

    int GenNonModifyMeta();
    int SortByPrefix();

private:
    std::string GetObjectLogFile(std::string &key);
    bool FindObjectInPrefixFile(std::string &key, std::string &operType);
    bool IsExistNewKey();
    void PushDirToWriteQueue(DirectoryScan &node, const Module::DirMetaWrapper &dirWrapper);
    int FillDirMetaWrapperByMetaFile(uint16_t metaId, uint64_t metaOffset, Module::DirMetaWrapper &dmWrapper);
    int FillFileMetaWrapperByMetaFile(uint16_t metaId, uint64_t metaOffset, Module::FileMetaWrapper &fmWrapper);
    int ReadXMeta(uint64_t xMetaId, uint64_t xMetaOffset, std::vector<Module::XMetaField> &xMeta);
    int SetDirDcanNode(DirectoryScan &dirNode, Module::FileCache& fcache);
    void CleanFileRes();
    int ReadFcache(Module::DirCache &dcache);
    int ReadDcache(std::string &metaDir);
    bool ValidateCurrFileCount();
    bool OpenAllFcacheMetaFiles(std::string &metaDir);
    void CloseScanMetaFiles();
    void ClearDiffThreads();
    void Poll();

    bool NeedHandleObject(std::string& key);
    int HandleLogKey(std::string &key, std::string &operType);

private:
    ScanConfig m_config {};
    std::shared_ptr<BufferQueue<DirectoryScan>> m_output;
    std::shared_ptr<StatisticsMgr> m_statsMgr;

    std::string m_logFileDirPath;
    std::string m_subPrefixRootDir;
    std::string m_delimiter;
    bool m_logFileExist { true };
    std::map<std::string, std::shared_ptr<SubPrefixParser>> m_filePaths {};

    std::shared_ptr<MetadataStat> m_scanMetaStat;
    std::shared_ptr<BufferQueue<Module::DirCache>> m_input {};
    std::vector<std::shared_ptr<ObjectMetaReadThread>> m_readThreads {};
};

#endif // FS_SCANNER_OBJECT_LOG_PROCESS_H
