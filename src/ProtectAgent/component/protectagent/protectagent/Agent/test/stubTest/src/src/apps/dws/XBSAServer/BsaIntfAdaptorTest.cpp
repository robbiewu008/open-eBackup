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
#include "apps/dws/XBSAServer/BsaIntfAdaptorTest.h"
#include "apps/dws/XBSAServer/xbsa_types.h"
#include "common/ConfigXmlParse.h"
namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_int32 StubReturnResultSetFailed(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_FAILED;
}
mp_int32 StubReturnResultSetSuc(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    strValue = "111";
    return MP_FAILED;
}
}

TEST_F(BsaIntfAdaptorTest, ABsaU64ToU64Test)
{
    BsaUInt64 b64;
    BsaIntfAdaptor::BsaU64ToU64(b64);
}

TEST_F(BsaIntfAdaptorTest, AU64ToBsaU64Test)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    mp_uint64 u64 = 1;
    BsaUInt64 b64;
    BsaIntfAdaptor::U64ToBsaU64(u64, b64);
}

TEST_F(BsaIntfAdaptorTest, HandleValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    int64_t handle;
    BsaIntfAdaptor::HandleValid(handle);
}

TEST_F(BsaIntfAdaptorTest, StringValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    int64_t handle;
    std::string str;
    mp_uint64 maxLen;
    mp_bool canBeEmpty;
    mp_bool bRet = BsaIntfAdaptor::StringValid(str, maxLen, canBeEmpty);
    EXPECT_EQ(bRet, MP_FALSE);

    str = "test";
    BsaIntfAdaptor::StringValid(str, maxLen, canBeEmpty);
    EXPECT_EQ(bRet, MP_FALSE);

    maxLen =  5;
    BsaIntfAdaptor::StringValid(str, maxLen, canBeEmpty);
    EXPECT_EQ(bRet, MP_SUCCESS);
}

TEST_F(BsaIntfAdaptorTest, BsaObjectOwnerValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    std::string bsaObjectOwner;
    BsaIntfAdaptor::BsaObjectOwnerValid(bsaObjectOwner);
}

TEST_F(BsaIntfAdaptorTest, AppObjectOwnerValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    std::string bsaObjectOwner;
    BsaIntfAdaptor::AppObjectOwnerValid(bsaObjectOwner);
}

TEST_F(BsaIntfAdaptorTest, ObjectSpaceNameValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    std::string bsaObjectOwner;
    BsaIntfAdaptor::ObjectSpaceNameValid(bsaObjectOwner);
}

TEST_F(BsaIntfAdaptorTest, PathNameValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    std::string pathName;
    BsaIntfAdaptor::PathNameValid(pathName);
}

TEST_F(BsaIntfAdaptorTest, ResourceTypeValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    std::string resourceType;
    BsaIntfAdaptor::ResourceTypeValid(resourceType);
}

TEST_F(BsaIntfAdaptorTest, ObjectDescriptionValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    std::string objectDescription;
    BsaIntfAdaptor::ObjectDescriptionValid(objectDescription);
}

TEST_F(BsaIntfAdaptorTest, ObjectInfoValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    std::string objectInfo;
    BsaIntfAdaptor::ObjectInfoValid(objectInfo);
}

TEST_F(BsaIntfAdaptorTest, CopyTypeValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    int32_t copyType;
    mp_bool canBeAny = MP_FALSE;
    mp_bool bRet = BsaIntfAdaptor::CopyTypeValid(copyType, canBeAny);
    EXPECT_EQ(bRet, MP_FALSE);
    
    copyType = BSA_CopyType_ARCHIVE;
    bRet = BsaIntfAdaptor::CopyTypeValid(copyType, canBeAny);
    EXPECT_EQ(bRet, MP_TRUE);

    copyType = BSA_CopyType_BACKUP;
    bRet = BsaIntfAdaptor::CopyTypeValid(copyType, canBeAny);
    EXPECT_EQ(bRet, MP_TRUE);

    copyType = BSA_CopyType_ANY;
    bRet = BsaIntfAdaptor::CopyTypeValid(copyType, canBeAny);
    EXPECT_EQ(bRet, MP_FALSE);
}

TEST_F(BsaIntfAdaptorTest, ObjectTypeValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    int32_t objectType = MP_FALSE;
    mp_bool canBeAny = MP_FALSE;
    mp_bool bRet = BsaIntfAdaptor::ObjectTypeValid(objectType, canBeAny);
    EXPECT_EQ(MP_TRUE, bRet);

    objectType = BSA_ObjectType_ANY;
    bRet = BsaIntfAdaptor::ObjectTypeValid(objectType, canBeAny);
    EXPECT_EQ(MP_FALSE, bRet);

    objectType = BSA_ObjectType_FILE;
    bRet = BsaIntfAdaptor::ObjectTypeValid(objectType, canBeAny);
    EXPECT_EQ(MP_TRUE, bRet);

    objectType = BSA_ObjectType_DIRECTORY;
    bRet = BsaIntfAdaptor::ObjectTypeValid(objectType, canBeAny);
    EXPECT_EQ(MP_TRUE, bRet);

    objectType = BASBSA_ObjectType_OTHER;
    bRet = BsaIntfAdaptor::ObjectTypeValid(objectType, canBeAny);
    EXPECT_EQ(MP_TRUE, bRet);
}

TEST_F(BsaIntfAdaptorTest, ObjectStatusValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    int32_t objectStatus;
    mp_bool canBeAny = MP_FALSE;
    mp_bool bRet = BsaIntfAdaptor::ObjectStatusValid(objectStatus, canBeAny);
    EXPECT_EQ(MP_FALSE, bRet);

    objectStatus = BSA_ObjectStatus_MOST_RECENT;
    bRet = BsaIntfAdaptor::ObjectStatusValid(objectStatus, canBeAny);
    EXPECT_EQ(MP_TRUE, bRet);

    objectStatus = BSA_ObjectStatus_NOT_MOST_RECENT;
    bRet = BsaIntfAdaptor::ObjectStatusValid(objectStatus, canBeAny);
    EXPECT_EQ(MP_TRUE, bRet);

    objectStatus = BSA_ObjectStatus_ANY;
    bRet = BsaIntfAdaptor::ObjectStatusValid(objectStatus, canBeAny);
    EXPECT_EQ(MP_FALSE, bRet);
}

TEST_F(BsaIntfAdaptorTest, VoteValidTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    int32_t vote;
    BsaIntfAdaptor::VoteValid(vote);
}

TEST_F(BsaIntfAdaptorTest, ConvertCreateReqObjTest)
{
    DoGetJsonStringTest();
    BsaObjectDescriptor src;
    BsaObjInfo dst;
    BSA_ObjectOwner sessionOwner;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetSuc);
    BsaIntfAdaptor::ConvertCreateReqObj(src, dst, sessionOwner);

    BsaObjectOwner objectOwner;
    objectOwner.bsaObjectOwner = "test";
    src.objectOwner = objectOwner;
    BsaIntfAdaptor::ConvertCreateReqObj(src, dst, sessionOwner);
}

TEST_F(BsaIntfAdaptorTest, ConvertCreateRspObjTest)
{
    DoGetJsonStringTest();
    BsaObjInfo src;
    BsaObjectDescriptor dst;
    BSA_ObjectOwner sessionOwner;
    BsaIntfAdaptor::ConvertCreateRspObj(src, dst, sessionOwner);
    
    BsaObjectOwner objectOwner;
    objectOwner.bsaObjectOwner = "test";
    dst.objectOwner = objectOwner;
    BsaIntfAdaptor::ConvertCreateRspObj(src, dst, sessionOwner);
}

TEST_F(BsaIntfAdaptorTest, ConvertQueryReqObjTest)
{
    DoGetJsonStringTest();
    BsaQueryDescriptor src;
    BsaObjInfo dst;
    BSA_ObjectOwner sessionOwner;
    mp_long sessionId;
    BsaIntfAdaptor::ConvertQueryReqObj(sessionId, src, dst, sessionOwner);

    BsaObjectOwner objectOwner;
    objectOwner.bsaObjectOwner = "test";
    src.objectOwner = objectOwner;
    BsaIntfAdaptor::ConvertQueryReqObj(sessionId, src, dst, sessionOwner);
}

TEST_F(BsaIntfAdaptorTest, ConvertQueryRspObjTest)
{
    DoGetJsonStringTest();
    BsaObjInfo src;
    BsaObjectDescriptor dst;
    BsaIntfAdaptor::ConvertQueryRspObj(src, dst);
}
