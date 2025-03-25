/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: g00554214
 * Create: 10/7/2020.
 */
#ifndef FS_SCANNER_SCAN_CONFIG_H
#define FS_SCANNER_SCAN_CONFIG_H
#include <set>
#include <memory>
#include "ScanConsts.h"
#ifdef _OBS
#include "common/CleanMemPwd.h"
#include "manager/CloudServiceManager.h"
#endif
#ifdef _NAS
#include "SmbContextArgs.h"

using Module::SmbContextArgs;
#endif
struct LibnfsConfig {
    std::string m_serverIp;
    std::string m_serverPath;
    std::string m_mountPath;
    uint8_t m_nasServerCheckSleepTime = 3;
    uint8_t m_contextCount = 8;
    uint16_t maxOpendirReqCount = 4000;
    uint16_t opendirReqCount = 0;
    uint16_t minOpendirReqCount = 0;
};

struct RfiCbStruct {
    std::string jobId;
    std::string subJobId;
    std::string copyId;
    std::string rfiZipFileName;
    bool isComplete;
    bool isFailed;
    bool isLastScan;
};

#ifdef _OBS
struct ObjectStorageBucket {
    std::string bucketName;
    std::vector<std::string> prefix;
    std::string delimiter {"/"};
    std::string logDir;
    uint32_t prefixSplitDepth = 0;
};

/*
 * 待备份的对象储存信息如下：
 * bucket1:{prefix1,prefix2}
 * bucket2:{prefix3}
 */
struct ObjectStorageConfig {
    Module::StorageConfig authArgs;
    std::vector<ObjectStorageBucket> buckets;
    int onceListNum = 1000; // 单次列举对象的数量，取值范围为1~1000
    bool IncUseLog = false; // 增量扫描使用对象储存的日志进行

    ~ObjectStorageConfig()
    {
        Module::CleanMemoryPwd(authArgs.verifyInfo.accessKey);
        Module::CleanMemoryPwd(authArgs.verifyInfo.secretKey);
        Module::CleanMemoryPwd(authArgs.verifyInfo.proxyUserName);
        Module::CleanMemoryPwd(authArgs.verifyInfo.proxyUserPwd);
    } // 删除敏感信息
};
#endif

struct ErrRecorder {
    int linuxErrCode = 0;
    std::string errCode;
    std::string errMessage;
};

// used for scan
enum FILTER_TYPE {
    INCLUDE, // means "include only"
    EXCLUDE,
    DISABLED // automatically set when list is empty
};

// used for control generation
enum class CtrlFilterType {
    INCLUDE, // means "include only"
    EXCLUDE
};


struct ScanFileFilter {
    FILTER_TYPE type = FILTER_TYPE::DISABLED;
    std::vector<std::string> fileList {};
};

struct ScanDirectoryFilter {
    FILTER_TYPE type = FILTER_TYPE::DISABLED;
    std::vector<std::string> dirList {};
};

class PathMapper {
public:
    virtual std::string Map(const std::string& path) = 0;           // origin master to snapshot path
    virtual std::string Recover(const std::string& path) = 0;       // snapshot path to origin path
};

// used for non-consist case
class DirectPathMapper : public PathMapper {
public:
    DirectPathMapper() = default;
    std::string Map(const std::string& path) override;
    std::string Recover(const std::string& path) override;
};

// used for scanning lvm snapshot directory to trim prefix
class PrefixSnapshotPathMapper : public PathMapper {
public:
    PrefixSnapshotPathMapper(const std::string& prefix);
    std::string Map(const std::string& path) override;
    std::string Recover(const std::string& path) override;
private:
    std::string     m_prefix;
};

/**
 * used for scanning solaris ZFS snapshot directory to trim infix
 * mapped path  /home/cl/.zfs/snapshot/dir1/dir2
 * infix string         /.zfs/snapshot
 * pos                  |--(pos = 8)
 * origin path  /home/cl              /dir1/dir2
 */
class InfixSnapshotPathMapper : public PathMapper {
public:
    InfixSnapshotPathMapper(std::size_t pos, const std::string& infixString);
    std::string Map(const std::string& path) override;
    std::string Recover(const std::string& path) override;
private:
    std::size_t         m_pos;
    std::string         m_infixString;
};

struct ScanConfig {
    ScanConfig()
    {}
    ~ScanConfig()
    {}
    std::string jobId {};                                   /* Job Id */
    std::string subJobId;                                   /* subjob id */
    std::string copyId;                                     /* copyId */
    std::size_t reqID {};                                   /* Request Id */
    ScanJobType scanType {};                                /* refer ScanJobType */
    IOEngine scanIO {};                                     /* refer IOEngine */
    CtrlFilterType ctrlFilterType {CtrlFilterType::INCLUDE}; /* refer CtrlFilterType */
    std::string failureRecordRootPath;                      /* to record file/dir failed to scan */
    std::string metaPath {};                                /* Meta path to produce cache files */
    std::string metaPathForCtrlFiles {};                    /* Meta path to produce control files */
    std::string curDcachePath;                              /* Meta path of current dcache files */
    std::string prevDcachePath;                             /* Meta path of previous dcache files */
    std::string indexPath;                                  /* Meta path to produce RFI Files */
    std::string reportPath {};                              /* Report path for RFI */
    time_t lastBackupTime = {};                             /* Last FULL backup time */
    bool useLastBackupTime = true;                          /* Should use last backup time for INC backup? */
    bool scanCheckPointEnable = false;                       /* Is Checkpoint enabled? */
    bool isLastScan = false;                                /* used for index */
    bool isPre = false;                                     /* used for index */
    bool filterFlag = false;                                /* used for index */
    std::string chainId = "";                               /* used for index */
    bool scanSparseFile = true;                             /* Whether to scan sparse File? */
    bool scanExtendAttribute = true;                        /* Whether to scan extend attribute? */
    bool scanAcl = true;                                    /* Whether to scan Acl? */
    bool disableSmbAcl = false;
    bool disableSmbNlink = false;
    bool genDirOnly = false;                                /* only used in CONTROL_GEN, only gen ctrl file with dir */
    bool scanByRoot = false;                                /* force use uid = 0 , gid = 0, to scan */
    bool enableProduce = true;                              /* won't produce meta and control file once disabled */
    bool keepRfiFile = false;
    bool ignoreDir = false;                                 /* ignore dir when scan */
    bool scanAds = true;

    int metadataScanType = METABACKUP_TYPE_FILE_AND_FOLDER; /* Metadata scan type */
    bool isGetDirExtAcl = 0;                                /* Enable get extended ACLs for directory */
    bool isGetFileExtAcl = 0;                               /* Enable get extended ACLs for file */
    bool isArchive = false;                                 /* used for archive scan */
    SCAN_HASH_TYPE scanHashType = SHA_1;

    // path mapping rule to map between snapshot path and origin path, used for V2 interface
    std::shared_ptr<PathMapper> pathMapper { nullptr };
    bool enableV2 { false };

    std::vector<std::string> skipDirs {                     /* skip the directories that cannot backup */
        ".", "..", ".snapshot", "~snapshot"
    };

#ifdef _NAS
    /* NAS scanner config */
    LibnfsConfig nfs {};                                    /* libnfs config */
    SmbContextArgs smb {};                                  /* libsmb config */
#endif
#ifdef _OBS
    ObjectStorageConfig obs {}; /* 保存扫描对象储存的高级配置参数 */
    bool encodeEnable = false;
#endif

    void* usrData = nullptr;                                /* usrData for callback */
    std::function<void(void*, std::string)> scanResultCb;   /* CB API to provide scan results (ctrl file) */
    std::function<void(void*, std::string)> scanHardlinkResultCb;   /* CB API to provide hlink ctrl file) */
    std::function<void(void*, std::string)> mtimeCtrlCb;    /* CB API to provide mtime dir results (ctrl file) */
    std::function<void(void*, std::string)> deleteCtrlCb;    /* CB API to provide del files/dir results (ctrl file) */
    std::function<void(void*, RfiCbStruct cbParam)> rfiCtrlCb;
    uint32_t maxCommonServiceInstance = 1;                  /* Max writer thread */
    std::string scanCtrlMaxDataSize;                        /* Copy control file - max datasz in single file */
    std::string scanCtrlMinDataSize;                        /* Copy control file - min datasz in single file */
    uint32_t scanCtrlFileTimeSec = 0;                       /* Copy control file - produce atleast 1 file per XX sec */
    uint32_t scanCopyCtrlFileSize = 4194304;                /* Copy and hardlink control file size */
    uint32_t scanCtrlMaxEntriesFullBkup = 0;                /* Copy control file - max entry in single file for FULL */
    uint32_t scanCtrlMinEntriesFullBkup = 0;                /* Copy control file - min entry in single file for FULL */
    uint32_t scanCtrlMaxEntriesIncBkup = 0;                 /* Copy control file - max entry in single file for INC */
    uint32_t scanCtrlMinEntriesIncBkup = 0;                 /* Copy control file - min entry in single file for INC */
    uint32_t scanMtimeCtrlMaxEnties = 100000;               /* Mtime control file - max entry in single file */
    uint32_t scanDeleteCtrlMaxEnties = 100000;              /* Delete control file - max entry in single file */
    uint64_t scanMetaFileSize = 10000;                      /* Meta file - max size */
    uint64_t maxWriteQueueSize = 10000;                     /* max queue size reached scan will stop and wait */
    uint64_t maxScanQueueSize = 10000;                      /* max queue size */
    uint64_t minScanQueueSize = 8000;                       /* min queue size */
    uint64_t writeQueueSize = 1000;                         /* max write queue size */
    uint32_t dirEntryReadCount = 1024;                      /* max direntries read from opendir request */
    time_t triggerTime = {};                                /* Scanner start time */
    bool generatorIsFull;                                   /* generator generate full or inc */
    uint8_t maxServerRetryCnt = 3;                          /* Server check retry count */
    uint8_t diffThreadCount = 16;                           /* Diff thread count */
    uint8_t producerThreadCount = 32;                       /* Num of producer thread */
    ScanDirectoryFilter dFilter {};                         /* Filter fields for scan */
    ScanFileFilter fFilter {};
    std::vector<std::string> dCtrlFltr;                     /* used for finegrained restore in CONTROL_GEN */
    std::vector<std::string> fCtrlFltr;
    std::set<std::string> crossVolumeSkipSet {};            /* used for consistent backup */
    NAS_PROTOCOL nasSnapdiffProtocol {};                    /* fields that used for V5 snapdiff */
    std::string deviceResourceName;
    std::string deviceUrl;
    std::string devicePort;
    std::string deviceUsername;
    std::string devicePassword;
    std::string deviceCert;
    int devicePoolID { 0 };
    std::string baseSnapshotName;
    std::string incSnapshotName;
    uint64_t maxOpendirReqCount = 4000;                     /* delete this */
    time_t expiredSkipTime = 0;
    time_t expiredColdSkipTime = 0;
};

#endif