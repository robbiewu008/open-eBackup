#ifndef __DATA_PATH_VMWARENATIVE_DATAPATH_IMPL_H__
#define __DATA_PATH_VMWARENATIVE_DATAPATH_IMPL_H__

#include <vector>
#include <memory>
#include <pthread.h>
#include <sstream>
#include "common/Defines.h"
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "common/ErrorCode.h"
#include "dataprocess/vmwarenative/Define.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "dataprocess/datamessage/DataMessage.h"
#include "dataprocess/vmwarenative/VMwareDiskLib.h"
#include "dataprocess/vmwarenative/VMwareDiskApi.h"
#include "dataprocess/ioscheduler/IOTask.h"
#include "dataprocess/ioscheduler/TaskScheduler.h"
#include "dataprocess/ioscheduler/IOEngine.h"
#include "afs/AfsProcess.h"
#include "dataprocess/ioscheduler/VmfsIO.h"

typedef struct storage_lun_info_ {
    mp_string arrayVendor;
    mp_string lunId;
    mp_string wwn;
    mp_string arraySn;
    mp_string arrayVersion;
    mp_string arrayModel;
    mp_string deviceName;
    mp_string diskNumber;
} storage_lun;

// external class
class VMwareDiskApi;

class VMwareNativeDataPathImpl {
public:
    VMwareNativeDataPathImpl();
    ~VMwareNativeDataPathImpl();

    // common interfaces
    mp_int32 VMwareNativeVddkInit(Json::Value &bodyMsg, mp_bool vddkInited);
    mp_int32 VMwareNativeVddkCleanup(Json::Value &bodyMsg);
    mp_int32 VMwareNativePreparation(Json::Value &bodyMsg);
    mp_int32 TargetLunPrepare(Json::Value &bodyMsg);

    // vm backup interfaces
    mp_int32 VMwareNativeDataBlockBackup(Json::Value &bodyMsg, mp_string &strError);
    mp_int32 VMwareNativeBackupOpenDisk(Json::Value &bodyMsg);
    mp_int32 VMwareNativeBackupCloseDisk();
    mp_int32 BackupProgressQuery(Json::Value &bodyMsg);
    mp_int32 VMwareVmBackupCancel(Json::Value &bodyMsg);
    mp_int32 DataBlockBackupFinish(Json::Value &bodyMsg);
    mp_int32 VMwareVmBackupFinish(Json::Value &bodyMsg);

    // invalid data identify
    mp_int32 VMwareNativeBackupAfsBitmap(Json::Value &bodyMsg, mp_string &strError);

    // vm restore interfaces
    mp_int32 VMwareNativeDataBlockRestore(Json::Value &bodyMsg, mp_string &strError);
    mp_int32 RestoreProgressQuery(Json::Value &bodyMsg);
    mp_int32 VMwareVmRestoreCancel(Json::Value &bodyMsg);
    mp_int32 DataBlockRestoreFinish(Json::Value &bodyMsg);
    mp_int32 VMwareVmRestoreFinish(Json::Value &bodyMsg);

    void ProcessDiskBlocks();
    mp_int32 ProcessDiskBlocksStart(
        TaskScheduler &ts, mp_int32 &nTasks, bool &failure, mp_int32 &jobThrsNum, mp_uint64 &totalSize);
    mp_int32 ProcessDiskBlocksStartForAioDatamover(
            TaskScheduler &ts, mp_int32 &nTasks, bool &failure, mp_int32 &jobThrsNum, mp_uint64 &totalSize);
    mp_int32 ProcessDiskBlocksStartForAioDatamoverForBackup(
            TaskScheduler &ts, mp_int32 &nTasks, bool &failure, mp_int32 &jobThrsNum, mp_uint64 &totalSize);
    mp_int32 ProcessDiskBlocksStartForAioDatamoverForRestore(
            TaskScheduler &ts, mp_int32 &nTasks, bool &failure, mp_int32 &jobThrsNum, mp_uint64 &totalSize);
    mp_void ProcessDiskBlocksEnd(TaskScheduler &ts, mp_int32 &nTasks, bool &failure);
    void StopThreads();
    void DoRmVmUuidFile(Json::Value &msgBody);
    mp_int32 CheckPathParm(std::string &path);

private:
    // data read/write operations
    EXTER_ATTACK mp_int32 OpenDisk(mp_uint32 openMode);
    void OpenDiskPrintLog();
    // close disk
    mp_int32 CloseDisk();
    mp_bool GetUseAioRunTask();
    static void *ProtectDiskDataBlock(mp_void *arg);
    static void *ExecReadFun(mp_void *arg);
    static void *ExecWriteFun(mp_void *arg);
    void *ReadVMwareDiskFun(mp_int32 type);
    void *WriteDataLunFun(mp_int32 type);
    void *ReadDataLunFun(mp_int32 type);
    void *WriteVMwareDiskFun(mp_int32 type);

    bool IsRDMDisk();
    void ScanDiskByIscsiAndFc();

    // parse storage lun
    mp_int32 ParseStorageLunMounted(Json::Value &msg);
    mp_int32 GetLocalDevicePathByWwn(const mp_string &wwn, mp_string &localDevicePath);
    mp_int32 MatchSpecificTargetLun(mp_string &wwn);
    mp_int32 ParseDirtyRangesParams(Json::Value &msg);
    mp_int32 ParseProtectEnvParams(Json::Value &msg, vmware_pe_info &peInfo);
    mp_int32 JsonArray2DirtyRangeArr(const std::vector<Json::Value> &jsonArr, std::vector<dirty_range> &dirtyRanges);
    mp_int32 JsonArray2StorageLunArr(const std::vector<Json::Value> &jsonArr, std::vector<storage_lun> &lunList);
    mp_int32 PkgJsonBody(Json::Value &jsonMsgBody);
    mp_int32 ParseVolumeParams(Json::Value &msg);
    mp_int32 ParseDiskType(Json::Value &msg, mp_string &diskType);

    // qos
    mp_int32 UpdateSubTaskQos();

    // parse vm protection params
    mp_int32 InitVmProtectionParams(Json::Value &msg);
    mp_int32 ParsePreparationRequsetParams(
        const Json::Value &msg, mp_uint64 &snapType, vmware_vm_info &vmInfo, vmware_pm_info &pmInfo);
    mp_int32 InitVddkLibPath(Json::Value &msg, mp_string &vddkPath);
    mp_int32 TryConnecetWithSpecifiedMode();
    void GetTransportMode(const bool& enableAdvanced, mp_string& transportMode);
    mp_int32 TryToOpenDisk(
        const mp_uint32 &openMode, const mp_uint64 &chunkSize, mp_string &tryTransMode, mp_string &errDesc);

    // 执行数据块处理前的准备工作
    mp_int32 PrepareProcess(Json::Value &bodyMsg, mp_string &errDesc);
    mp_int32 PrepareProcessPreJob(Json::Value &bodyMsg, mp_string &errDesc);
    // 获取任务需要的线程数
    mp_int32 GetJobThreadNum(int jobType);
    // 计算任务执行进度
    void CalcJobProgress(mp_int32& progress, mp_int32& jobStatus, mp_string& jobDesc);
    // 执行单个数据块的处理任务
    bool ExecIOTask(
        TaskScheduler &ts, std::shared_ptr<IOTask> &task, mp_int32 &nTasks, bool &failure, mp_int32 taskThrsNum);
    // 打开任务的读写流
    mp_int32 OpenRW();
    // 关闭任务的读写流
    mp_int32 CloseRW();

    // 检测指定的WWN是否处于挂载
    bool CheckDiskIsMappedByWwn();
    void CheckHotAddVmdkDevice(Json::Value &bodyMsg);

    mp_int32 ParseStorageType(const Json::Value &msgBody);
    mp_int32 ParseVmSnapshotRef(const Json::Value &msgBody);
    mp_int32 InitVddkLib(Json::Value &msgBody, mp_bool &isInited);
    bool GetConfigDataFromDataProcessNode(mp_uint64& chunkSize, mp_bool& isEnableSSL, mp_bool& isEnableAdvanced);

    // 计算速度
    mp_uint64 GetDiskSpeed(mp_int32 progress);
    void InnerCleanup();
    mp_int32 VerifyParamsSize(const mp_uint64 &param, const mp_uint64 &size, const mp_string &name);
    mp_int32 CheckVmwareVmInfo(vmware_vm_info &params);
    mp_int32 CheckVmwarePmInfo(vmware_pm_info &params);
    mp_int32 CheckVmwareVolumeInfo(vmware_volume_info &params);
    std::shared_ptr<IOEngine> CreateVmfsIOEngine();
    // 无效数据识别 BEGIN
    void FilterDiskBitmap();
    void FilterAfsBitmapForBackup(AfsProcess &afs, DiskHandleInfo &info);
    void BuildConnectParams(VddkConnectParams &connectParams);
    void JsonArray2SpecifiedArr(const Json::Value &jsonVal, std::vector<std::string> &specifiedLst);
    mp_int32 JsonArray2ExcludeDataArr(const Json::Value &jsonVal, DataExclusion &excludeInfos);
    mp_int32 ParseExcludeDataParams(const Json::Value &msg, DataExclusion &info);

    mp_int32 JsonArray2AfsVolumeArr(const std::vector<Json::Value> &jsonArr, std::vector<FilterDIiskInfo> &volumes);
    mp_int32 ParseSnapshotRef(const Json::Value &jsonMsgBody);
    mp_int32 ParseAfsVolumesInfo(const Json::Value &msg, std::vector<FilterDIiskInfo> &volumes);
    void FillVmwareTmpVolumeInfo(const FilterDIiskInfo &afsVolParams);
    bool FillDiskHandleParams(
        const FilterDIiskInfo &info, const std::shared_ptr<VMwareDiskApi> &api, DiskHandleInfo &handle);

    mp_int32 TryToOpenDiskForAfsBitmap(const std::shared_ptr<VMwareDiskApi> &diskApi, const mp_uint32 &openMode,
        const mp_uint64 &chunkSize, mp_string &tryTransMode, mp_string &errDesc);
    mp_int32 OpenDiskForAfsBitmapInner(std::shared_ptr<VMwareDiskApi> &diskApi);

    mp_int32 GetDevicePathForRdm();
    mp_int32 OpenDisksForAfsBitmap(
        const std::vector<FilterDIiskInfo> &infos, std::vector<DiskHandleInfo> &vecDiskHandle);

    void CloseAllDisksForAfsBitmap(const std::vector<DiskHandleInfo> &vecDiskHandle);

    void PreFillDiskHandleParams(DiskHandleInfo &handle);
    // END

private:
    mp_int32 PrivateParsePreparationRequsetParams(
        const Json::Value &msg, vmware_pm_info &pmInfo);
    mp_string m_strTaskID;
    mp_string m_strParentTaskID;
    std::shared_ptr<VMwareDiskApi> m_spVMwareDiskApi;

    // each task has its own protection params
    vmware_protection_params m_vmProtectionParams;
    // each task has its own vddk conn params
    VddkConnectParams m_vddkConnParams;

    vmware_volume_info m_volumeInfo;
    mp_string m_localDevicePath;
    mp_string m_transportModeSelected;
    std::vector<storage_lun> m_lunList;

    mp_int32 m_progress;
    mp_int64 m_LastTimePoint;
    mp_int64 m_iDirtyRangeSize;
    mp_uint64 m_nowSpeed;
    mp_uint64 m_preDataSizeFinished;
    mp_uint64 m_preTime;
    mp_uint32 m_snapType;
    mp_bool m_isQosLimitSpeed;

    // protection type: 0-backup, 1-recovery
    mp_int32 m_protectType;
    // backend storage type: 0-fusionstorage, 1-nas
    mp_uint32 m_backendStorageProtocol;

    mp_bool m_isDiskOpened;
    std::atomic<std::uint64_t> m_completedBlocks; // 记录任务完成的数据块数量
    std::atomic<std::uint64_t> m_zeroBlocks; // 记录任务完成时0数据块数量
    std::atomic<std::uint64_t> m_totalCompletedBlocks; // 记录任务完成的总数据块数量
    std::shared_ptr<IOEngine> m_reader; // 任务的读端，如备份任务读端为VMware，恢复任务则读端为存储
    std::shared_ptr<IOEngine> m_writer; // 任务的写端，如备份任务写端为存储，恢复任务则写端为VMware
    std::shared_ptr<NSVmfsIO::VmfsIO> m_vmfsio; // 存储层备份vmfsio

    std::atomic<std::uint64_t> m_reducedBlocks;  // 记录无效数据识别减少的数据块数量

    // 需要join的线程列表，这些线程使用了该类对象的资源，需要在类对象析构前等待线程退出
    // 要求对应的线程需要设置线程对取消信号的响应，以便能cancel掉该线程
    // 设置方式为pthread_setcancelstate,pthread_setcanceltype
    std::list<std::thread> m_joinableThrs;

    // 记录任务状态，等close线程结束后才设置任务100%，才等待DME接收新的dirty range
    std::atomic<mp_bool> m_isClose;

    // IO task exec status
    std::atomic<mp_bool> m_isTaskSuccess;

    // thread exit flag
    std::atomic<mp_bool> m_exitFlag;

    // hostagent system virt type
    mp_int32 m_systemVirt;

    std::mutex m_mutex;

    mp_string m_errDesc = "failure";
    mp_string m_diskType;
};

#endif