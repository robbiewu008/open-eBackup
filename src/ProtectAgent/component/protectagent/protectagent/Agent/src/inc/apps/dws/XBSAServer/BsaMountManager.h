#ifndef _BSA_MOUNT_MANAGER_H_
#define _BSA_MOUNT_MANAGER_H_

#include <map>
#include <mutex>
#include <vector>
#include "common/Types.h"
#include "apps/dws/XBSAServer/DwsTaskCommonDef.h"

struct FsMountInfo {
    mp_uint16 allocIndex{0}; // 分配挂载点的索引
    mp_string deviceSN;
    mp_string fsId;
    mp_string fsName;
    std::vector<mp_string> mountPathList; // 文件系统不同逻辑端口的挂载点列表
};

class BsaMountManager {
public:
    BsaMountManager() {};
    ~BsaMountManager() {};

    mp_void SetRepository(const mp_string &taskId, const std::vector<DwsRepository> &repos);
    mp_void AllocFilesystem(const mp_string &taskId, mp_string &deviceSN, mp_string &fsId, mp_string &fsName);
    mp_string GetMountPath(const mp_string &taskId, const mp_string &deviceSN, const mp_string &fsName);
    mp_bool IsFsMounted(const mp_string &taskId, const mp_string &deviceSN, const mp_string &fsId,
                        const mp_string &fsName);
    static BsaMountManager &GetInstance()
    {
        return m_instance;
    }
    mp_void ClearMountInfoByTaskId(const mp_string &taskId);

private:
    mp_void AddIntoMountedList(const mp_string &taskId, const mp_string &deviceSN, const DwsFsInfo &fsInfo,
                            std::vector<FsMountInfo> &output);
    std::map<mp_string, mp_uint16> m_allocFsIndex; // 分配文件系统的索引.key是任务ID
    std::mutex m_mutexMountFsList;
    std::map<mp_string, std::vector<FsMountInfo>> m_mountedFsList; // 已挂载成功的文件系统列表。key是任务ID
    static BsaMountManager m_instance;
};

#endif // _BSA_MOUNT_MANAGER_H_