#ifndef __AGENT_VMWARENATIVE_DEFINE_H__
#define __AGENT_VMWARENATIVE_DEFINE_H__

const static int VMWARE_DATABLOCK_SIZE = 4194304;  // Byte
const static int VMWARE_DATABLOCK_SIZE_MB = 4;  // MB
static const mp_int32 THREAD_NUMS = 2;

enum vSphereErrorCode {
    vSphereErrorCodeOK = 0,
    vSphereErrorCodeInternalError,
    vSphereErrorCodeNetworkError,
    vSphereErrorCodeNotAuthenticated,
    vSphereErrorCodeInvalidLogin,
    vSphereErrorCodeManagedObjectNotFound,
};

enum eDMVMWareErrCode {
    DMVMWareErrorCodeInternalError = 0x60E10B00,
    DMVMWareErrorCodeInvalidParameter,
    DMVMWareErrorCodeTaskNotExists,
    DMVMWareErrorCodeTaskAlreadyExists,
    DMVMWareErrorCodeSnapInitFailed,
    DMVMWareErrorCodeAllocateMemoryFailed,
    DMVMWareErrorCodeBackupStoragePathFormatError,
    DMVMWareErrorCodeBlockNotBackuped,
    DMVMWareErrorCodeGetTaskInfoFailed,
    DMVMWareErrorCodeTaskNotFinish,
    DMVMWareErrorCodeChainIsBusying,
    DMVMWareErrorCodeDownloadChainDBFailed,
    DMVMWareErrorCodeCheckHealthStatusFailed
};

struct vSphereInfo {
    vSphereInfo()
    {}
    ~vSphereInfo()
    {
        for (std::string::iterator it = password.begin(); it != password.end(); ++it) {
            *it = 0;
        }
    }
    std::string ip;
    std::string port;
    std::string userName;
    std::string password;
    std::string protocol;
    std::string cert;
};

typedef struct disk_datablock {
    disk_datablock() : index(0), size(0), dataBuff("")
    {}
    disk_datablock(mp_uint64 index, mp_uint64 size, const mp_string& buff)
    {
        this->index = index;
        this->size = size;
        this->dataBuff = buff;
    }
    mp_uint64 index;
    mp_uint64 size;
    mp_string dataBuff;
} st_disk_datablock;

enum vmware_vm_protection_status {
    TASK_NOT_EXIST = 0,
    TASK_NOT_STARTED,
    TASK_RUNNING,
    TASK_RUN_FAILURE,
    TASK_RUN_SUCCESS,
    TASK_DELETING,
    TASK_DELETED,
    TASK_DELETE_FAILURE
};

enum vmware_vm_protection_type {
    VMWARE_VM_BACKUP = 0,
    VMWARE_VM_RECOVERY,
    VMWARE_VM_NFS_BACKUP
};

enum vmware_hostagent_system_virt {
    PHYSICAL_MACHINE = 0,
    VIRTUAL_MACHINE
};

#endif