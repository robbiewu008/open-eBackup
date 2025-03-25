/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: Prouce meta&xmeta for object storage.
* Author: w00444223
* Create: 2023-12-04
*/

#ifndef FS_SCANNER_OBJECT_META_PRODUCER_H
#define FS_SCANNER_OBJECT_META_PRODUCER_H

#include <dirent.h>
#include "ObjectUtils.h"
#include "MetaProducer.h"
#include "ScanCommon.h"
#include "manager/CloudServiceManager.h"

class ObjectMetaProducer : public MetaProducer, public ObjectUtils {
public:
    explicit ObjectMetaProducer(ProduceParams args, ScanConfig config)
        : MetaProducer(args.scanQueue, args.output, args.statsMgr, args.scanFilter, args.chkPntMgr), m_config(config)
    {};
    ObjectMetaProducer() {};
    ~ObjectMetaProducer() override {};

    SCANNER_STATUS InitContext() override;
    SCANNER_STATUS DestroyContext() override;
    void Produce(int count) override;

private:
    std::unique_ptr<Module::CloudServiceInterface> m_obsCtx {nullptr};
    std::vector<std::pair<DirectoryScan, Module::DirMetaWrapper>> m_subDirNode {};

    void ProcessCurDir(DirStat &dirStat);
    void PushDirToWriteQueue(DirectoryScan& node, const Module::DirMetaWrapper dirWrapper);

    int GetBucketACL(std::string &bucketName, std::string &aclText);
    int GetObjectACL(std::string &bucketName, std::string &key, std::string &aclText);
    int GetObjectMetaData(std::string &bucketName, std::string &key, ObjectMetaData &metaDataInfo);

    int FillFileMetaWrapper(FileStat& fileStat, Module::FileMetaWrapper &fileWrapper);
    int WrapDirectory(Module::DirMetaWrapper &dirWrapper, DirStat& dirStat);
    int WrapFile(Module::FileMetaWrapper &fileWrapper, struct FileStat& fileStat);

    void SetFileStat(FileStat &fileStat, std::string &key, std::string etag, uint64_t lastModified, uint64_t size);
    void HandleSubPrefix(
        const std::string &prefix, const std::string &parentPath, const std::vector<std::string> &commonPrefixes);
    ObjectStorageBucket GetObsBucketConfig(std::string dirPath);

    // Search for the position of Objcet in ScanInProgressList and erase Object if found in list
    bool SkipDirEntry(const std::string &name, const std::string &fullPath) const;
    void FillListObjectsReq(const std::unique_ptr<Module::ListObjectsRequest>& req);
    void ScanDirectory(Module::DirMetaWrapper& dirWrapper, DirStat& dirStat);

    int ReadDirEntry(const struct dirent *direntry, DirStat &dirStat, Module::DirMetaWrapper &dirWrapper,
        std::string &parentPath);
    void ScanObjectLogContent(Module::DirMetaWrapper& dirWrapper, DirStat& dirStat);
    bool NeedHandleObject(std::string& bucketPrefix, std::string& delimiter, uint32_t maxDepth, std::string& key);
    std::string ReplaceSlashWithDelimiter(std::string &path);
    int HandleSubPrefixFile(std::string pathName, Module::DirMetaWrapper &dirWrapper, std::string &parentPath);
    void HandleUserPrefix(Module::DirMetaWrapper& dirWrapper, DirStat& dirStat);
    void HandleScannedDir(Module::DirMetaWrapper& dirWrapper, DirStat& dirStat,
        DirectoryScan& subDirNode, uint64_t obsTotal);
    int HandleObjectContent(std::unique_ptr<Module::ListObjectsResponse>& resq, bool isTruncated);
    bool FindFirstDifference(const std::string comparedpath);
    size_t MakeDirStat(std::string path);
    bool HandleLastPath();
    std::string GetBucketPath(std::string key);
    void GetParentPrefix(std::string prefix, std::set<std::string>& parentPrefix);
    void CreateWrapDir(Module::DirMetaWrapper &dirWrapper, DirStat& dirStat);
    void CreateNewNode(std::string path);

    int HandleFileStat(DirectoryScan& subDirNode, Module::DirMetaWrapper& dirWrapper,
    FileStat &fileStat, uint32_t& obsCnt);

private:
    ScanConfig m_config {};
    bool m_curDirIsPushed { false };
    ObjectStorageBucket m_bucket;
    std::string m_lastPath;
    std::unordered_map<std::string, DirectoryScan> m_directoryMap;
    std::shared_ptr<std::vector<Module::ListObjectsContent>> m_listedObjects  {nullptr};
    std::string m_fullPath;
    std::string m_scanPreFix;
};
#endif