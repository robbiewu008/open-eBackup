#ifndef _BSA_DB_H_
#define _BSA_DB_H_

#include "common/DB.h"
#include "apps/dws/XBSAServer/BsaObjManager.h"

struct DwsHostInfo {
    DwsHostInfo() {}
    DwsHostInfo(const mp_string &host, const mp_string &id, const mp_string &name, const mp_string &deviceId)
        : hostname(host), fsId(id), fsName(name), fsDeviceId(deviceId) {}
    mp_string hostname;
    mp_string fsId;
    mp_string fsName;
    mp_string fsDeviceId;
};

class BsaDb {
public:
    BsaDb(const mp_string &dbFile) : m_dbFile(dbFile) {};
    ~BsaDb() {};

    mp_int32 CreateBsaObjTable();
    mp_int32 InsertBsaObj(const BsaObjInfo &obj);
    mp_int32 QueryBsaObjs(const BsaObjInfo &queryCond, const BsaQueryPageInfo &pageInfo,
        std::vector<BsaObjInfo> &objList);

    // 事务操作，需要在调用处加锁
    mp_int32 BeginTrans();
    mp_int32 RollbackTrans();
    mp_int32 CommitTrans();

    mp_int32 CreateDwsHostFilesystemTable();
    mp_int32 InsertDwsHost(const DwsHostInfo &host);
    mp_int32 DeleteDwsHost(const mp_string &hostname);
    mp_int32 QueryDwsHosts(const BsaQueryPageInfo &pageInfo, std::vector<DwsHostInfo> &hostList);

private:
    mp_void TransObjParam(DBReader &readBuff, BsaObjInfo &obj);
    mp_int32 QueryObject(const mp_string &sql, DbParamStream &dps, std::vector<BsaObjInfo> &objList);
    mp_void BuildQueryCond(const BsaObjInfo &queryCond, mp_string &sql, DbParamStream &dps);

    mp_void TransHostParam(DBReader &readBuff, DwsHostInfo &host);

    mp_string m_dbFile;
};

#endif // _BSA_DB_H_