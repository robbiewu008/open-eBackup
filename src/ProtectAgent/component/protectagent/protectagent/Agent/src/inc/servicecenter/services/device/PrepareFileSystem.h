#ifndef _PREPARE_FILE_SYSTEM_H
#define _PREPARE_FILE_SYSTEM_H

#include <vector>
#include <set>
#include <mutex>
#include "common/Utils.h"
#include "common/Types.h"
#include "common/Defines.h"
#include "common/ErrorCode.h"


namespace AppProtect {
#ifdef WIN32
    const mp_string MOUNT_PUBLIC_PATH = GetSystemDiskChangedPathInWin("C:\\mnt\\databackup\\");
#else
    // E6000设备即时挂载，需要扫描副本目录用于创建文件克隆，写入扫描结果时，需要提权到root执行
    // root执行会校验目录，修改时需要同步修改ROOT_COMMAND_WRITE_SCAN_RESULT命令字的定义
    const mp_string MOUNT_PUBLIC_PATH = "/mnt/databackup/";
    const mp_string MOUNT_PUBLIC_PATH_PREFIX = "/mnt/";
#endif

    struct MountPermission {
        mp_string uid;
        mp_string gid;
        mp_string mode;
        std::vector<mp_string> vecPath;
    };

    struct MountNasParam {
        ~MountNasParam()
        {
            if (!authPwd.empty()) {
                ClearString(authPwd);
            }
            if (!cifsAuthPwd.empty()) {
                ClearString(cifsAuthPwd);
            }
        }
        mp_string jobID;
        mp_string repositoryType;
        std::vector<mp_string> vecStorageIp;
        std::vector<mp_string> vecDataturboIP;
        mp_string storagePath;
        mp_string storageName;
        mp_string appType = "general_type";
        mp_string runAccount;
        bool isFullPath = true;
        MountPermission permit;
        mp_string authKey;
        mp_string authPwd;
        mp_string cifsAuthKey;
        mp_string cifsAuthPwd;
        mp_bool isLinkEncryption = false;
        mp_bool isFcOn = false;
        mp_bool isDeduptionOn = false;
        mp_bool isFileClientMount = false;
        mp_string esn;
        mp_int32 nCopyFormat = 1; // CopyFormatType 0=snapshot, 1=directory
        mp_string protocolType;
        mp_string lunInfo;
        bool useMemSave {true};
        bool isCloneFsMount {true};

        mp_string ip;
        mp_string mountPath;
        mp_string subPath;
        mp_string mountOption;
        mp_string mountProtocol;
        mp_string pluginName;
        mp_int32 OSADAuthPort = 0;
        mp_int32 OSADServerPort = 0;
        mp_string pvcTaskId;
        mp_int32 pvcOSADAuthPort = 0;
        mp_int32 pvcOSADServerPort = 0;
        mp_int32 taskType = 0; // Default taskType=UNDEFINE
    };

    struct MountNasKeyParam {
        MountNasKeyParam(const MountNasParam& mountNasParam)
        {
            jobID = mountNasParam.jobID;
            storageName = mountNasParam.storageName;
            authKey = mountNasParam.authKey;
            cifsAuthKey = mountNasParam.cifsAuthKey;
            storagePath = mountNasParam.storagePath;
            authPwd = mountNasParam.authPwd;
            cifsAuthPwd = mountNasParam.cifsAuthPwd;
            useMemSave = mountNasParam.useMemSave;
            repositoryType = mountNasParam.repositoryType;
            appType = mountNasParam.appType;
            isCloneFsMount = mountNasParam.isCloneFsMount;
        }
        ~MountNasKeyParam()
        {
            ClearString(authPwd);
            ClearString(cifsAuthPwd);
        }
        mp_string jobID;
        mp_string storageIp;
        mp_string storagePath;
        mp_string storageName;
        mp_string authKey;
        mp_string authPwd;
        mp_string cifsAuthKey;
        mp_string cifsAuthPwd;
        bool isInerSnapshot {false};
        mp_string command;
        mp_string strSubPath;
        bool useMemSave {true};
        mp_string repositoryType;
        mp_string appType;
        bool isCloneFsMount {true};
    };

    struct MountChainInfo {
        MountChainInfo(const MountNasParam& mountNasParam);

        mp_string repositoryType;
        mp_string storageIp;
        mp_string protocolType;
        mp_string protocolPort;
        mp_string sharePath;
    };

class PrepareFileSystem {
public:
    PrepareFileSystem()
    {}
    ~PrepareFileSystem()
    {}

    EXTER_ATTACK mp_int32 MountNasFileSystem(MountNasParam &mountNasParam,
        std::vector<mp_string> &successMountPath);
    EXTER_ATTACK  mp_int32 MountDataturboFileSystem(MountNasParam &mountNasParam,
        std::vector<mp_string> &successMountPath, std::vector<mp_string> &dtbMountPath);
    EXTER_ATTACK mp_int32 MountFileIoSystem(MountNasParam &mountNasParam,
        std::vector<mp_string> &successMountPath, const mp_string &recordFile);
    EXTER_ATTACK mp_int32 MountFileClientSystem(
        MountNasParam &mountNasParam, std::vector<mp_string> &successMountPath, std::vector<mp_string> &realMountPath);

    mp_int32 GetMountParam(
        const MountNasParam &mountNasParam, mp_string &mountOption, mp_string &mountProtocol, mp_string &pluginName);
    mp_string GetNasMountScriptParama(const MountNasParam &mountNasParam);
    mp_int32 GetPluginDefinedMountOption(const MountNasParam &mountNasParam, mp_string &mountOption);
    mp_int32 UmountNasFileSystem(const std::vector<mp_string> &successMountPath, const mp_string &jobId = "",
        const bool &isFileClientMount = false);
    mp_int32 CreateLocalFileDir(mp_string localPath);

    mp_void DeleteRepoTempDir(const mp_string &repoType, const mp_string &tempDir)
    {
        m_umountRepoType = repoType;
        m_repoTempDir = tempDir;
    }

    static bool IsMountChainGood(const mp_string& jobId);      // it is used when read or write fail
private:
    mp_string GetMountPublicPath();
    mp_string GenerateSubPath(const MountNasParam &mountNasParam);
    mp_string SplitFileClientMountPath(const mp_string &mountPath);
    mp_void GetMountPath(const MountNasParam &mountNasParam, const mp_string iterIp, mp_string &mountPath);
    mp_int32 CheckAndCreateDataturboLink(const mp_string &storageName, const MountNasParam &mountNasParam);
    bool GetMountOption(const MountNasParam &mountNasParam, mp_string &mountOptionKey, mp_string &mountOption);

#ifdef WIN32
    mp_int32 WinMountOperation(MountNasKeyParam& mountKeyParam, mp_string& mountPath);
    mp_int32 WinMountOperationInner(const MountNasKeyParam& mountKeyParam, mp_string& driveLetter);
    mp_string GetWinMountScriptParama(const MountNasKeyParam &mountKeyParam);
    mp_int32 WinMountUseScript(const MountNasKeyParam& mountKeyParam, mp_string& driveLetter);
    mp_int32 WinMountUseAPI(const MountNasKeyParam& mountKeyParam, mp_string& driveLetter);
    mp_bool CreateLink(const mp_string &linkPath, const mp_string &targetPath);
    mp_void SplitMountPath(const mp_string &strPath, std::map<mp_string, mp_string> &mapMountPath);
    mp_int32 WinUmountOperation(const std::vector<mp_string> &successMountPath, const mp_string &jobId);
#endif

    void AddMountInfoToMap(const mp_string& mountInfo, const mp_string& mountPath, bool needSave = true);
    bool QueryMountInfoFromMap(const mp_string& mountInfo, mp_string& mountPath, bool needSave = true);
    static void EraseMountInfoFromMap(const mp_string& mountPath);
    static std::map<mp_string, mp_string> m_alreadyMountInfoMap;
    static std::mutex m_mountInfoMapMutex;

    mp_string m_umountRepoType;
    mp_string m_repoTempDir;
    mp_string m_linkPath;

    static void JobOccupyMountPoint(const mp_string& mountPoint, const mp_string& jobId);
    static void JobReleaseMountPoint(const mp_string& mountPoint, const mp_string& jobId);
    static bool IsMountPointOccupied(const mp_string& mountPoint);
    static std::map<mp_string, std::set<mp_string>> m_mountPointOccupiedInfoMap;
    static std::mutex m_mountPointMapMutex;

    // adjust ip list order, put ip which mount fail at the end
    static void AdjustMountIpList(std::vector<mp_string>& mountIpList);
    // record mount fail ip when mount fail
    static void AddMountFailIp(const mp_string& ip);
    // mount success when use ip recored, release it
    static void ReleaseMountFailIp(const mp_string& ip = "");

    static std::map<mp_string, uint64_t> m_mountFailIpMap;    // ip, record time
    static std::mutex m_mountFailIpMapMutex;

    void HandleAfterMountSuccess(const MountNasParam &mountNasParam);
    void HandleAfterMountFail(const MountNasParam &mountNasParam, mp_int32 &errCode);
    void HandleAfterUMountSuccess(const mp_string& jobId, const mp_string& mountPath);

    static void JobAddMountChainInfo(const mp_string& jobId, const MountChainInfo& mountChainInfo);
    static void JobReleaseMountChainInfo(const mp_string& jobId);

    // job mount chain info
    static std::map<mp_string, std::vector<MountChainInfo>> m_mountChainInfoMap;       // jobid, chainList
    static std::mutex m_mountChainMapMutex;
};
}  // namespace AppProtect

#endif