#include "common/ConfigXmlParse.h"
#include "common/Path.h"
#include "apps/dws/XBSAServer/BsaObjManagerTest.h"
#include "apps/dws/XBSAServer/BsaSessionManager.h"
#include "apps/dws/XBSAServer/BsaDb.h"

namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}

#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)
}

/*
* 测试用例：CreateObject接口测试
* 前置条件：无
* CHECK点：1.函数返回成功,2.objectStatus等于BSA_ObjectStatus_MOST_RECENT
*/
TEST_F(BsaObjManagerTest, CreateObject_Test)
{
    DoGetJsonStringTest();
    BsaObjInfo objInfo;
    EXPECT_EQ(BsaObjManager::GetInstance().CreateObject(objInfo), MP_SUCCESS);
    EXPECT_EQ(objInfo.objectStatus, BSA_ObjectStatus_MOST_RECENT);
}

static mp_string GetAggregatedBsaDbFilePath_Fail()
{
    return "";
}
/*
* 测试用例：QueryObject接口测试
* 前置条件：无
* CHECK点：1.获取db文件名为空时返回失败.2.db文件不存在时返回失败.
*/
TEST_F(BsaObjManagerTest, QueryObject_Test)
{
    DoGetJsonStringTest();

    // db文件不存在时返回失败
    BsaObjInfo queryCond;
    BsaQueryPageInfo pageInfo;
    std::vector<BsaObjInfo> result;
    mp_long bsaHandle = 12345;
    EXPECT_EQ(BsaObjManager::GetInstance().QueryObject(queryCond, pageInfo, result, bsaHandle), MP_FAILED);

    // 获取db文件名为空时返回失败
    EXPECT_EQ(BsaObjManager::GetInstance().QueryObject(queryCond, pageInfo, result, bsaHandle), MP_FAILED);
}

static mp_string GetBsaDbFilePathh_Fail()
{
    return "";
}

static mp_string GetBsaDbFilePathh_Succ()
{
    cout << "GetDwsHostDbFilePath_Stub" << endl;
    return "/tmp/dwsObj.db";
}

/*
* 测试用例：SaveObjects接口测试
* 前置条件：无
* CHECK点：1.获取db文件名为空时返回失败.2.db文件不存在时返回失败.3.db文件存在时保存对象到数据库成功
*/
TEST_F(BsaObjManagerTest, SaveObjects_Test)
{
    DoGetJsonStringTest();
    BsaObjInfo obj;
    std::map<mp_uint64, BsaObjInfo> objList;
    obj.objectSpaceName = "roach";
    obj.bsaObjectOwner = "roach";
    obj.appObjectOwner = "owner";
    obj.copyType = 2;
    obj.resourceType = 4;
    obj.objectType = 5;
    obj.objectDescription = "1234";
    obj.objectInfo = "";
    obj.timestamp = "2022.3.0";
    obj.restoreOrder = 10086;
    obj.objectStatus = 11;
    obj.copyId = 100;
    obj.objectName = "/opt/log";
    obj.estimatedSize = 20;
    obj.storePath = "/data/xxxx111/opt/log";
    obj.fsId = "11";
    obj.fsName = "xwjwh";
    obj.fsDeviceId = "2133swqwewq3334";
    objList[100] = obj;

    mp_long bsaHandle = 12345;
    // 获取db文件名为空时返回失败
    stub.set(ADDR(BsaSessionManager, GetBsaDbFilePath), GetBsaDbFilePathh_Fail);
    EXPECT_EQ(BsaObjManager::GetInstance().SaveObjects(objList, bsaHandle), MP_FAILED);

    // db文件不存在时返回失败
    stub.set(ADDR(BsaSessionManager, GetBsaDbFilePath), GetBsaDbFilePathh_Succ);
    EXPECT_EQ(BsaObjManager::GetInstance().SaveObjects(objList, bsaHandle), MP_FAILED);

    // db文件存在时保存对象到数据库成功
    system("touch /tmp/dwsObj.db");
    mp_string dbFile = GetBsaDbFilePathh_Succ();
    BsaDb db(dbFile);
    EXPECT_EQ(db.CreateBsaObjTable(), MP_SUCCESS);
    EXPECT_EQ(BsaObjManager::GetInstance().SaveObjects(objList, bsaHandle), MP_SUCCESS);

    // 资源清理
    system("rm -f /tmp/dwsObj.db");
}