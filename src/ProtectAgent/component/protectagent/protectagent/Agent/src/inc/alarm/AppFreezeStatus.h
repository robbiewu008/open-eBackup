#ifndef _APP_FREEZE_STATUS_H_
#define _APP_FREEZE_STATUS_H_

#include "common/Types.h"
#include "common/DB.h"
#include <vector>

typedef struct freeze_status_st {
    mp_string strKey;  // 挂载点路径或其他能标识冻结解冻单元的key
    mp_int32 iStatus;
} freeze_status;

class AppFreezeStatus {
public:
    static mp_int32 Insert(const freeze_status& stStatus);
    static mp_int32 Delete(const freeze_status& stStatus);
    static mp_void Get(freeze_status& stStatus);
    static mp_int32 GetAll(std::vector<freeze_status>& vecStatus);

private:
    static mp_bool IsExist(const freeze_status& stStatus);
};

#endif
