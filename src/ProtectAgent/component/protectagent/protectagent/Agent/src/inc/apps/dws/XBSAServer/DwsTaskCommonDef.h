#ifndef DWS_TASK_COMMON_DEF
#define DWS_TASK_COMMON_DEF

#include <vector>
#include "common/Types.h"
#include "common/JsonHelper.h"

enum class DwsTaskType {
    BACKUP = 0,
    RESTORE = 1,
    DELETE = 2,
};

enum class DwsRepoRole {
    MASTER = 0,
    SLAVE = 1,
};

enum class XbsaJobType {
    FULL_BACKUP = 0,
    DIFF_BACKUP = 1,
    FULL_RESTORE = 2,
    DIFF_RESTORE = 3
};

struct DwsCacheInfo {
    mp_string cacheRepoPath; // cache仓文件系统的本地挂载路径
    mp_string metaRepoPath; // meta仓文件系统的本地挂载路径，归档到云直接恢复时插件给的是cache仓的挂载地址
    mp_string copyId; // 当前任务对应的副本Id
    mp_string taskId; // 当前正在进行的任务Id
    mp_string hostKey; // 本节点在整个DWS集群内唯一的key，目前使用的是rdagent注册到PM的IP

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(cacheRepoPath, cacheRepoPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(metaRepoPath, metaRepoPath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(copyId, copyId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(taskId, taskId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(hostKey, hostKey)
    END_SERIAL_MEMEBER
};

struct DwsFsInfo {
    mp_string id;
    mp_string name;
    mp_string sharePath;
    std::vector<mp_string> mountPath;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(sharePath, sharePath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountPath, mountPath)
    END_SERIAL_MEMEBER
};

struct DwsRepository {
    mp_uint32 role{0};
    mp_string deviceSN; // 存储设备的ESN，唯一标识一套设备
    std::vector<DwsFsInfo> filesystems;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(role, role)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceSN, deviceSN)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(filesystems, filesystems)
    END_SERIAL_MEMEBER
};

struct ArchiveFileServerInfo {
    mp_string ip;
    mp_uint32 port{0};
    bool sslEnabled{true};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(port, port)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(sslEnabled, sslEnabled)
    END_SERIAL_MEMEBER
};

struct DwsTaskInfo {
    mp_uint32 taskType{static_cast<mp_uint32>(DwsTaskType::BACKUP)};
    mp_uint32 copyType{0}; // 副本类型，与agent thrift接口定义的enum CopyDataType一致
    std::vector<DwsRepository> repositories;
    std::vector<ArchiveFileServerInfo> fileServers;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(taskType, taskType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(copyType, copyType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(repositories, repositories)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fileServers, archiveFileServers)
    END_SERIAL_MEMEBER
};

struct DwsXbsaSpeedInfo {
    uint64_t speedInMBps{0};
    uint64_t totalSizeInMB{0};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(speedInMBps, speedInMBps)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(totalSizeInMB, totalSizeInMB)
    END_SERIAL_MEMEBER
};

struct FsRelation {
    mp_uint32 role{0};
    mp_string oldEsn;
    mp_string oldFsName;
    mp_string oldFsId;
    mp_string newEsn;
    mp_string newFsName;
    mp_string newFsId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(role, role)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(oldEsn, oldEsn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(oldFsName, oldFsName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(oldFsId, oldFsId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(newEsn, newEsn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(newFsName, newFsName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(newFsId, newFsId)
    END_SERIAL_MEMEBER
};

struct DwsFsRelation {
    std::vector<FsRelation> relations;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(relations, relations)
    END_SERIAL_MEMEBER
};

struct XbsaBusinessConfig {
    mp_uint32 jobType{static_cast<mp_uint32>(XbsaJobType::FULL_RESTORE)};
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(jobType, jobType)
    END_SERIAL_MEMEBER
};

struct FsKeyInfo {
    FsKeyInfo() {}
    FsKeyInfo(const mp_string &id, const mp_string &name, const mp_string &deviceId)
        : fsId(id), fsName(name), fsDeviceId(deviceId) {}
    mp_string fsId;
    mp_string fsName;
    mp_string fsDeviceId; // 文件系统对应的设备Id(ESN)
    bool operator == (const FsKeyInfo &other) const
    {
        return (fsId == other.fsId && fsName == other.fsName && fsDeviceId == other.fsDeviceId);
    }
    bool operator < (const FsKeyInfo &other) const
    {
        if (fsDeviceId != other.fsDeviceId) {
            return (fsDeviceId < other.fsDeviceId);
        }
        if (fsName != other.fsName) {
            return (fsName < other.fsName);
        }
        return (fsId < other.fsId);
    }
};

struct BsaObjInfo {
    mp_string objectSpaceName;
    mp_string objectName;
    mp_string bsaObjectOwner;
    mp_string appObjectOwner;
    mp_uint32 copyType{0};
    mp_uint64 estimatedSize{0};
    mp_string resourceType;
    mp_uint32 objectType{0};
    mp_string objectDescription;
    mp_string objectInfo;
    mp_string timestamp;
    mp_uint64 copyId{0};
    mp_uint64 restoreOrder{0};
    mp_uint32 objectStatus{0};
    mp_string storePath;
    mp_string fsId;
    mp_string fsName;
    mp_string fsDeviceId; // 文件系统对应的设备Id(ESN)

    int getDataType{0};
    mp_string archiveBackupId;
    mp_string archiveServerIp;
    int archiveServerPort{0};
    int archiveOpenSSL{0}; // 1:open  0:close
};

#endif