#ifndef DWS_TASK_MANAGE_H_
#define DWS_TASK_MANAGE_H_
#include "apps/dws/XBSAServer/AppTaskManage.h"

class DwsTaskManage : public AppTaskManage {
public:
    DwsTaskManage() : AppTaskManage() {}
    mp_int32 UpdateTaskWhenCreateObject(const BsaObjectDescriptor &objDesc) override;
    mp_int32 UpdateTaskWhenQueryObject(const BsaQueryDescriptor &objDesc) override;
    mp_void AllocFilesystem(BsaObjInfo &objInfo) override;
private:
    mp_int32 GetTaskInfoLockedInner();
    mp_int32 GetDwsHosts();
    mp_string GetDwsHostDbFilePath();
    mp_string ParseDwsHostname(const mp_string &objName);
    mp_void UpdateDwsHosts();
private:
    std::map<mp_string, FsKeyInfo> m_dwsHostMap;
};

#endif