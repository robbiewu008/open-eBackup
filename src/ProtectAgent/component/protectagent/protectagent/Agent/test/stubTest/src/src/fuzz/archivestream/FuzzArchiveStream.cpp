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
#include "fuzz/archivestream/FuzzArchiveStream.h"
#include "secodeFuzz.h"
#include "Cmd.h"
#include "message/archivestream/ArchiveStreamService.h"
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#ifndef WIN32
#include <sys/time.h>
#endif
#include "securecom/RootCaller.h"

using namespace std;

namespace {
const mp_string MANAGECMD_KEY_CMDNO = "cmd";
const mp_string MANAGECMD_KEY_BODY = "body";

const mp_string RECOVERYMESSAGE_OFFSET = "offset";
const mp_string RECOVERYMESSAGE_ARCHIVE_BACK_UP_ID = "archiveBackupId";
const mp_string RECOVERYMESSAGE_FILE_PATH = "filePath";
const mp_string RECOVERYMESSAGE_READ_SIZE = "readSize";
const mp_string RECOVERYMESSAGE_MAX_RESPONSE_SIZE = "maxResponseSize";
const mp_string RECOVERYMESSAGE_ARCHIVE_BACKUP_ID = "backupID";
const mp_string RECOVERYMESSAGE_DIRS = "dirs";
const mp_string RECOVERYMESSAGE_STATE = "state";
const mp_string RECOVERYMESSAGE_DIR_COUNT = "dirCount";
const mp_string RECOVERYMESSAGE_FILE_COUNT = "dirCount";
const mp_string RECOVERYMESSAGE_BACKUP_FILE_SIZE = "backupFileSize";
const mp_string RECOVERYMESSAGE_READ_COUNT_LIMIT = "readCountLimit";
const mp_string RECOVERYMESSAGE_OBJECT_LIST = "objectList";
const mp_string RECOVERYMESSAGE_OBJECT_NAME = "objectName";
const mp_string RECOVERYMESSAGE_OBJECT_FSID = "fsid";
const mp_string RECOVERYMESSAGE_OBJECT_IS_DIR = "isDir";
const mp_string RECOVERYMESSAGE_STATUS = "status";
const mp_string RECOVERYMESSAGE_SPLIT_FILE = "splitFile";
const mp_string RECOVERYMESSAGE_OBJECT_NUM = "objectNum";
const mp_string RECOVERYMESSAGE_METADATA = "metadata";
const mp_string RECOVERYMESSAGE_CHECKPOINT = "checkpoint";

const std::string DEFAULT_MOUNT_PATH = "/mnt/databackup/archivestream/";
const mp_string RECOVERYMESSAGE_FS_SHARE_NAME = "fsShareName";
const mp_string RECOVERYMESSAGE_META_FILE_DIR = "metaFileDir";
const mp_string RECOVERYMESSAGE_LOGIC_IPV4_LIST = "logicIpv4List";
const mp_string RECOVERYMESSAGE_LOGIC_IPV6_LIST = "logicIpv6List";
const mp_string RECOVERYMESSAGE_AGENT_IP_LIST = "agentIpList";

const mp_int32 DPP_MSG_TIMEOUT = 300;
const mp_int32 DEFULAT_SEND_TIMEOUT = 3;  // send time out, default 3 seconds
const mp_int32 ARCHIVE_STREAM_FORMAT_STRING = 512;
const mp_int32 ARCHIVE_STREAM_FORMAT_UUID = 36;
const mp_int32 ARCHIVE_STREAM_FORMAT_UINT32 = 4294967295;
const mp_int32 ARCHIVE_STREAM_FORMAT_UINT64 = 512;
const mp_int32 ARCHIVE_STREAM_FORMAT_PORT = 65535;

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_void StubDoSleepTest100ms(mp_uint32 ms)
{
#ifdef WIN32
    Sleep(100);
#else
    struct timeval stTimeOut;
    const mp_int32 timeUnitChange = 100000;
    stTimeOut.tv_sec  = 100 / timeUnitChange;
    stTimeOut.tv_usec = (100 % timeUnitChange) * timeUnitChange;
    (mp_void)select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}

mp_int32 SendDppMsg_Prepare_stub(const mp_string &taskId, const Json::Value &strReqMsg, Json::Value &strRspMsg, mp_uint32 reqCmd)
{
    char strInit[] = "1234";
    Json::Value reqBody;
    reqBody[RECOVERYMESSAGE_FS_SHARE_NAME] = DT_SetGetString(&g_Element[0], 5, ARCHIVE_STREAM_FORMAT_STRING, strInit);
    strRspMsg[MANAGECMD_KEY_BODY] = reqBody;
    return MP_SUCCESS;
}

mp_int32 StubSUCCESS(mp_void* pThis)
{
    return MP_SUCCESS;
}
}

TEST_F(FuzzArchiveStream, PrepareRecoveryTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(ArchiveStreamService, SendDppMsg), SendDppMsg_Prepare_stub);
    m_stub.set(ADDR(ArchiveStreamService, CheckIPLinkStatus), StubSUCCESS);

    ArchiveStreamService stArchiveStreamService;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzPrepareRecovery";
    DT_FUZZ_START1(strDtFuzzName)
    {
        mp_string input;
        EXPECT_EQ(MP_FAILED, stArchiveStreamService.PrepareRecovery(input));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzArchiveStream, QueryPrepareStatusTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(ArchiveStreamService, SendDppMsg), SendDppMsg_Prepare_stub);

    ArchiveStreamService stArchiveStreamService;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzQueryPrepareStatus";
    DT_FUZZ_START1(strDtFuzzName)
    {
        mp_int32 input;
        EXPECT_EQ(MP_SUCCESS, stArchiveStreamService.QueryPrepareStatus(input));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzArchiveStream, GetBackupInfoTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(ArchiveStreamService, SendDppMsg), SendDppMsg_Prepare_stub);

    ArchiveStreamService stArchiveStreamService;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzGetBackupInfo";
    DT_FUZZ_START1(strDtFuzzName)
    {
        ArchiveStreamCopyInfo input;
        EXPECT_EQ(MP_SUCCESS, stArchiveStreamService.GetBackupInfo(input));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzArchiveStream, GetRecoverObjectListTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(ArchiveStreamService, SendDppMsg), SendDppMsg_Prepare_stub);

    ArchiveStreamService stArchiveStreamService;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzGetRecoverObjectList";
    DT_FUZZ_START1(strDtFuzzName)
    {
        s64 dt_fuzz_int_init = 1;
        mp_string splitFile; 
        mp_int64 objectNum; 
        mp_int32 status;
        char strInit[] = "1234";
        mp_string checkpoint;
        checkpoint = DT_SetGetString(&g_Element[0], 5, ARCHIVE_STREAM_FORMAT_STRING, strInit);
        mp_int64 readCountLimit = *(s64*)DT_SetGetS64(&g_Element[1], dt_fuzz_int_init);
        EXPECT_NE(MP_TRUE, stArchiveStreamService.GetRecoverObjectList(readCountLimit, checkpoint, splitFile, objectNum, status));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzArchiveStream, GetDirMetaDataTest)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(ArchiveStreamService, SendDppMsg), SendDppMsg_Prepare_stub);

    ArchiveStreamService stArchiveStreamService;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzGetDirMetaData";
    DT_FUZZ_START1(strDtFuzzName)
    {
        char strInit[] = "1234";
        mp_string ObjectName;
        mp_string fsID; 
        mp_string MetaData;

        ObjectName = DT_SetGetString(&g_Element[0], 5, ARCHIVE_STREAM_FORMAT_STRING, strInit);
        fsID = DT_SetGetString(&g_Element[1], 5, ARCHIVE_STREAM_FORMAT_STRING, strInit);
        EXPECT_EQ(MP_SUCCESS, stArchiveStreamService.GetDirMetaData(ObjectName, fsID, MetaData));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzArchiveStream, GetFileMetaData)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(ArchiveStreamService, SendDppMsg), SendDppMsg_Prepare_stub);

    ArchiveStreamService stArchiveStreamService;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzGetFileMetaData";
    DT_FUZZ_START1(strDtFuzzName)
    {
        char strInit[] = "1234";
        mp_string ObjectName;
        mp_string fsID; 
        mp_string MetaData;

        ObjectName = DT_SetGetString(&g_Element[0], 5, ARCHIVE_STREAM_FORMAT_STRING, strInit);
        fsID = DT_SetGetString(&g_Element[1], 5, ARCHIVE_STREAM_FORMAT_STRING, strInit);
        EXPECT_EQ(MP_SUCCESS, stArchiveStreamService.GetFileMetaData(ObjectName, fsID, MetaData));
    }
    DT_FUZZ_END()
}

TEST_F(FuzzArchiveStream, Connect_Fail)
{
    m_stub.set(&CLogger::Log, StubCLoggerLog);
    m_stub.set(ADDR(ArchiveStreamService, SendDppMsg), SendDppMsg_Prepare_stub);
    ArchiveStreamService stArchiveStreamService;
    DT_Enable_Leak_Check(0,0);
    char strDtFuzzName[] = "FuzzGetFileMetaData";
    DT_FUZZ_START1(strDtFuzzName)
    {
        std::string busiIp = "8.40.0.0";
        int busiPort = 1008;
        bool openSsl {false};
        int ret = stArchiveStreamService.Connect(busiIp, busiPort, openSsl);
        EXPECT_EQ(ret, MP_FAILED);
    }
    DT_FUZZ_END()
}
