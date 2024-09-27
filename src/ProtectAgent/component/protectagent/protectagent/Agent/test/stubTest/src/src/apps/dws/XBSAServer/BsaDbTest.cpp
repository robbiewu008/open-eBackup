/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "apps/dws/XBSAServer/BsaDbTest.h"
#include "xbsa/xbsa.h"

namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
        stub.set(ADDR(CLogger, Log), LogTest); \
    } while (0)

mp_int32 StubFailed(mp_void *pvoid)
{
    return MP_FAILED;
}
}

/*
* 测试用例：创建BsaObjTable表
* 前置条件：无
* CHECK点：1.表不存在时创建成功，已存在时创建成功
*/
TEST_F(BsaDbTest, CreateBsaObjTableTest)
{
    DoGetJsonStringTest();

    system("touch /tmp/BsaDbTest.db");

    BsaDb db("/tmp/BsaDbTest.db");
    EXPECT_EQ(db.CreateBsaObjTable(), MP_SUCCESS); // 表不存在时创建成功
    EXPECT_EQ(db.CreateBsaObjTable(), MP_SUCCESS); // 已存在时创建成功

    system("rm -f /tmp/BsaDbTest.db");
}

/*
* 测试用例：插入和查询BsaObjTable表
* 前置条件：无
* CHECK点：1.表不存在时插入失败.2.存在时插入成功.3.插入后可以查询
*/
TEST_F(BsaDbTest, InsertBsaObjTest)
{
    DoGetJsonStringTest();

    BsaObjInfo obj;
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

    system("touch /tmp/BsaDbTest.db");

    BsaDb db("/tmp/BsaDbTest.db");
    EXPECT_NE(db.InsertBsaObj(obj), MP_FAILED); // 1.表不存在时插入失败
    EXPECT_EQ(db.CreateBsaObjTable(), MP_SUCCESS);
    EXPECT_EQ(db.InsertBsaObj(obj), MP_SUCCESS); // 表存在时插入成功

    BsaObjInfo queryCond;
    BsaQueryPageInfo pageInfo;
    std::vector<BsaObjInfo> objList;
    queryCond.objectName = "/opt/log";
    queryCond.copyType = obj.copyType;
    queryCond.objectType = obj.objectType;
    queryCond.objectStatus = obj.objectStatus;
    EXPECT_EQ(db.QueryBsaObjs(queryCond, pageInfo, objList), MP_SUCCESS); // 对象名称匹配
    EXPECT_EQ(objList.size(), 1);
    EXPECT_TRUE(obj.objectSpaceName == objList[0].objectSpaceName);
    EXPECT_TRUE(obj.bsaObjectOwner == objList[0].bsaObjectOwner);
    EXPECT_TRUE(obj.appObjectOwner == objList[0].appObjectOwner);
    EXPECT_TRUE(obj.copyType == objList[0].copyType);
    EXPECT_TRUE(obj.resourceType == objList[0].resourceType);
    EXPECT_TRUE(obj.objectType == objList[0].objectType);
    EXPECT_TRUE(obj.objectDescription == objList[0].objectDescription);
    EXPECT_TRUE(obj.objectInfo == objList[0].objectInfo);
    EXPECT_TRUE(obj.timestamp == objList[0].timestamp);
    EXPECT_TRUE(obj.restoreOrder == objList[0].restoreOrder);
    EXPECT_TRUE(obj.objectStatus == objList[0].objectStatus);
    EXPECT_TRUE(obj.copyId == objList[0].copyId);
    EXPECT_TRUE(obj.objectName == objList[0].objectName);
    EXPECT_TRUE(obj.estimatedSize == objList[0].estimatedSize);
    EXPECT_TRUE(obj.storePath == objList[0].storePath);
    EXPECT_TRUE(obj.fsId == objList[0].fsId);
    EXPECT_TRUE(obj.fsName == objList[0].fsName);
    EXPECT_TRUE(obj.fsDeviceId == objList[0].fsDeviceId);

    queryCond.objectSpaceName = "*";
    EXPECT_EQ(db.QueryBsaObjs(queryCond, pageInfo, objList), MP_SUCCESS); // 对象名称匹配，通配查询
    EXPECT_EQ(objList.size(), 1);

    queryCond.objectSpaceName = "";
    queryCond.bsaObjectOwner = "*";
    EXPECT_EQ(db.QueryBsaObjs(queryCond, pageInfo, objList), MP_SUCCESS); // 对象名称匹配，通配查询
    EXPECT_EQ(objList.size(), 1);

    queryCond.objectSpaceName = "";
    queryCond.bsaObjectOwner = "";
    queryCond.appObjectOwner = "*";
    EXPECT_EQ(db.QueryBsaObjs(queryCond, pageInfo, objList), MP_SUCCESS); // 对象名称匹配，通配查询
    EXPECT_EQ(objList.size(), 1);

    queryCond.objectSpaceName = "*";
    queryCond.bsaObjectOwner = "*";
    queryCond.appObjectOwner = "*";
    EXPECT_EQ(db.QueryBsaObjs(queryCond, pageInfo, objList), MP_SUCCESS); // 对象名称匹配，通配查询
    EXPECT_EQ(objList.size(), 1);

    queryCond.objectName = "/opt/log1";
    EXPECT_EQ(db.QueryBsaObjs(queryCond, pageInfo, objList), MP_SUCCESS); // 对象名称不匹配
    EXPECT_EQ(objList.size(), 0);

    queryCond.objectName = "/opt/log";
    pageInfo.limit = 10;
    pageInfo.offset = 2;
    EXPECT_EQ(db.QueryBsaObjs(queryCond, pageInfo, objList), MP_SUCCESS); // offset超出
    EXPECT_EQ(objList.size(), 0);

    system("rm -f /tmp/BsaDbTest.db");
}

/*
* 测试用例：BuildQueryCond接口功能测试
* 前置条件：无
* CHECK点：1.appObjectOwner,bsaObjectOwner,objectSpaceName为空或者"*"时不作为查询条件
*/
TEST_F(BsaDbTest, BuildQueryCondTest)
{
    DoGetJsonStringTest();
    BsaDb db("/tmp/BsaDbTest.db");
    BsaObjInfo queryCond;
    mp_string sql;
    DbParamStream dps;
    db.BuildQueryCond(queryCond, sql, dps);
    EXPECT_EQ(sql.find("appObjectOwner") == std::string::npos, true);
    EXPECT_EQ(sql.find("bsaObjectOwner") == std::string::npos, true);
    EXPECT_EQ(sql.find("objectSpaceName") == std::string::npos, true);

    sql = "";
    queryCond.appObjectOwner = "*";
    queryCond.bsaObjectOwner = "*";
    queryCond.objectSpaceName = "*";
    db.BuildQueryCond(queryCond, sql, dps);
    EXPECT_EQ(sql.find("appObjectOwner") == std::string::npos, true);
    EXPECT_EQ(sql.find("bsaObjectOwner") == std::string::npos, true);
    EXPECT_EQ(sql.find("objectSpaceName") == std::string::npos, true);

    sql = "";
    queryCond.appObjectOwner = "a";
    queryCond.bsaObjectOwner = "b";
    queryCond.objectSpaceName = "c";
    db.BuildQueryCond(queryCond, sql, dps);
    EXPECT_EQ(sql.find("appObjectOwner") != std::string::npos, true);
    EXPECT_EQ(sql.find("bsaObjectOwner") != std::string::npos, true);
    EXPECT_EQ(sql.find("objectSpaceName") != std::string::npos, true);
}

/*
* 测试用例：BuildQueryCond接口功能测试2
* 前置条件：无
* CHECK点：1.copyType,objectType,objectStatus为ANY时不作为查询条件
*/
TEST_F(BsaDbTest, BuildQueryCondTest2)
{
    DoGetJsonStringTest();
    BsaDb db("/tmp/BsaDbTest.db");
    BsaObjInfo queryCond;
    mp_string sql;
    DbParamStream dps;
    queryCond.copyType = BSA_CopyType_ANY;
    queryCond.objectType = BSA_ObjectType_ANY;
    queryCond.objectStatus = BSA_ObjectStatus_ANY;
    db.BuildQueryCond(queryCond, sql, dps);
    EXPECT_EQ(sql.find("copyType") == std::string::npos, true);
    EXPECT_EQ(sql.find("objectType") == std::string::npos, true);
    EXPECT_EQ(sql.find("objectStatus") == std::string::npos, true);

    sql = "";
    queryCond.copyType = BSA_CopyType_ANY + 1;
    queryCond.objectType = BSA_ObjectType_ANY + 1;
    queryCond.objectStatus = BSA_ObjectStatus_ANY + 1;
    db.BuildQueryCond(queryCond, sql, dps);
    EXPECT_EQ(sql.find("copyType") != std::string::npos, true);
    EXPECT_EQ(sql.find("objectType") != std::string::npos, true);
    EXPECT_EQ(sql.find("objectStatus") != std::string::npos, true);
}

/*
* 测试用例：DwsHost表创建和增、删、查询功能测试
* 前置条件：无
* CHECK点：1.创表接口可以重复调用.2.增删和查询符合预期
*/
TEST_F(BsaDbTest, DwsHostTableTest)
{
    DoGetJsonStringTest();
    BsaDb db("/tmp/BsaDbTest.db");
    system("touch /tmp/BsaDbTest.db");

    EXPECT_EQ(db.CreateDwsHostFilesystemTable(), MP_SUCCESS); // 首次创建表成功
    EXPECT_EQ(db.CreateDwsHostFilesystemTable(), MP_SUCCESS); // 表已存在创建也是成功

    DwsHostInfo hostInfo1("host1", "id1", "fs1", "esn1");
    EXPECT_EQ(db.InsertDwsHost(hostInfo1), MP_SUCCESS); // 插入host1成功

    DwsHostInfo hostInfo2("host2", "id2", "fs2", "esn1");
    EXPECT_EQ(db.InsertDwsHost(hostInfo2), MP_SUCCESS); // 插入host2成功
    {
        BsaQueryPageInfo pageInfo(100, 0);
        std::vector<DwsHostInfo> hostList;
        EXPECT_EQ(db.QueryDwsHosts(pageInfo, hostList), MP_SUCCESS); // 查询成功
        EXPECT_EQ(hostList.size(), 2); // 查询出2条记录
    }

    EXPECT_EQ(db.DeleteDwsHost("host1"), MP_SUCCESS); // 删除host1成功
    {
        BsaQueryPageInfo pageInfo(100, 0);
        std::vector<DwsHostInfo> hostList;
        EXPECT_EQ(db.QueryDwsHosts(pageInfo, hostList), MP_SUCCESS); // 查询成功
        EXPECT_EQ(hostList.size(), 1); // 查询出1条记录，查询结果与插入记录的值相等
        EXPECT_EQ(hostList[0].hostname == "host2", true);
        EXPECT_EQ(hostList[0].fsId == "id2", true);
        EXPECT_EQ(hostList[0].fsName == "fs2", true);
        EXPECT_EQ(hostList[0].fsDeviceId == "esn1", true);
    }

    system("rm -f /tmp/BsaDbTest.db"); // 清理资源
}

TEST_F(BsaDbTest, DwsHostTest)
{
    DwsHostInfo hostInfo1("host1", "id1", "fs1", "esn1");
    mp_string hostname = "host1";
    BsaDb db("/tmp/BsaDbTest.db");
    system("touch /tmp/BsaDbTest.db");

    stub.set(ADDR(DWSDB, ExecSql), StubFailed);
    EXPECT_EQ(db.CreateDwsHostFilesystemTable(), MP_FAILED); 
    EXPECT_EQ(db.InsertDwsHost(hostInfo1), MP_FAILED); 
    EXPECT_EQ(db.DeleteDwsHost(hostname), MP_FAILED); 
    
    stub.reset(ADDR(DWSDB, ExecSql));
    system("rm -f /tmp/BsaDbTest.db"); // 清理资源
}

