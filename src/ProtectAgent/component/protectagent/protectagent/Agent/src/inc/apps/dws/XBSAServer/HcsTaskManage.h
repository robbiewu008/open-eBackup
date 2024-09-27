#ifndef HCS_TASK_MANAGE_H_
#define HCS_TASK_MANAGE_H_
#include "apps/dws/XBSAServer/AppTaskManage.h"

class HcsTaskManage : public AppTaskManage {
public:
    HcsTaskManage() : AppTaskManage() {}
    mp_int32 UpdateTaskWhenCreateObject(const BsaObjectDescriptor &objDesc) override;
    mp_int32 UpdateTaskWhenQueryObject(const BsaQueryDescriptor &objDesc) override;
    mp_void AllocFilesystem(BsaObjInfo &objInfo) override;
    mp_bool FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, QueryObjectResult &rsp) override;
    mp_bool FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, GetNextQueryObjectResult &rsp) override;
protected:
    template<typename T>
    mp_bool FillQuryRspCommonHcs(mp_long bsaHandle, const BsaObjInfo &queryReslt, T &rsp);
private:
    mp_int32 GetTaskInfoLockedInner();
};

#endif