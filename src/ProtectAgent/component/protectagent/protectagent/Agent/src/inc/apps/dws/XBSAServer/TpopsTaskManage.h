#ifndef TPOPS_TASK_MANAGE_H_
#define TPOPS_TASK_MANAGE_H_
#include "apps/dws/XBSAServer/AppTaskManage.h"

class TpopsTaskManage : public AppTaskManage {
public:
    TpopsTaskManage(mp_string jobType) : AppTaskManage() { m_jobType = jobType; }
    mp_int32 UpdateTaskWhenCreateObject(const BsaObjectDescriptor &objDesc) override;
    mp_int32 UpdateTaskWhenQueryObject(const BsaQueryDescriptor &objDesc) override;
    mp_void AllocFilesystem(BsaObjInfo &objInfo) override;
    mp_bool FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, QueryObjectResult &rsp) override;
    mp_bool FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, GetNextQueryObjectResult &rsp) override;
protected:
    template<typename T>
    mp_bool FillQuryRspCommonTpops(mp_long bsaHandle, const BsaObjInfo &queryReslt, T &rsp);
private:
    mp_int32 GetTaskInfoLockedInner();
    mp_bool GetTpopsCacheInfoFile(mp_string &tpopsCacheInfoFile, mp_string &instanceId);
private:
    mp_string m_jobType;
};

#endif