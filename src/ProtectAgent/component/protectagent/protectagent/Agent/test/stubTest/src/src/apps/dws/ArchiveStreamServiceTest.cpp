#include "stub.h"
#include "gtest/gtest.h"
#include "message/archivestream/ArchiveStreamClientHandler.h"
#include "message/archivestream/ArchiveStreamService.h"

class ArchiveStreamServiceTest : public testing::Test {
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    Stub stub;
    ArchiveStreamService m_archiveSteamService;
};

mp_int32 Ret_Succ()
{
    return MP_SUCCESS;
}

mp_int32 Ret_Fail()
{
    return MP_FAILED;
}

bool Ret_True()
{
    return true;
}

bool Ret_False()
{
    return false;
}

TEST_F(ArchiveStreamServiceTest, GetFileDataT)
{
    ArchiveStreamGetFileReq getFileReq;
    ArchiveStreamGetFileRsq getFileRsp;
    getFileReq.readSize = 1;
    getFileReq.maxResponseSize = 1;
    getFileReq.fileOffset = 64;
    stub.set(ADDR(ArchiveStreamClientHandler, GetResponseMessage), Ret_True);
    int ret = m_archiveSteamService.GetFileData(getFileReq, getFileRsp);
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(ArchiveStreamServiceTest, QueryPrepareStatusT)
{
    mp_int32 state;
    stub.set(ADDR(ArchiveStreamClientHandler, GetResponseMessage), Ret_True);
    int ret = m_archiveSteamService.QueryPrepareStatus(state);
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(ArchiveStreamServiceTest, GetBackupInfoT)
{
    ArchiveStreamCopyInfo copyInfo;
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Succ);
    int ret = m_archiveSteamService.GetBackupInfo(copyInfo);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(ArchiveStreamServiceTest, GetBackupInfoF)
{
    ArchiveStreamCopyInfo copyInfo;
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Fail);
    int ret = m_archiveSteamService.GetBackupInfo(copyInfo);
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(ArchiveStreamServiceTest, GetRecoverObjectListT)
{
    mp_int64 readCountLimit = 65532;
    mp_string checkpoint;
    mp_string splitFile;
    mp_int64 objectNum;
    mp_int32 status;
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Succ);
    int ret = m_archiveSteamService.GetRecoverObjectList(readCountLimit, checkpoint, splitFile, objectNum, status);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(ArchiveStreamServiceTest, GetRecoverObjectListF)
{
    mp_int64 readCountLimit = 65532;
    mp_string checkpoint;
    mp_string splitFile;
    mp_int64 objectNum;
    mp_int32 status;
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Fail);
    int ret = m_archiveSteamService.GetRecoverObjectList(readCountLimit, checkpoint, splitFile, objectNum, status);
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(ArchiveStreamServiceTest, GetDirMetaDataT)
{
    mp_string ObjectName = "objname";
    mp_string fsID = "fsid";
    mp_string MetaData = "sdkjfosuiodjgbkl";
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Succ);
    int ret = m_archiveSteamService.GetDirMetaData(ObjectName, fsID, MetaData);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(ArchiveStreamServiceTest, GetDirMetaDataF)
{
    mp_string ObjectName = "objname";
    mp_string fsID = "fsid";
    mp_string MetaData = "sdkjfosuiodjgbkl";
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Fail);
    int ret = m_archiveSteamService.GetDirMetaData(ObjectName, fsID, MetaData);
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(ArchiveStreamServiceTest, GetFileMetaDataT)
{
    mp_string ObjectName = "objname";
    mp_string fsID = "fsid";
    mp_string MetaData = "sdkjfosuiodjgbkl";
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Succ);
    int ret = m_archiveSteamService.GetFileMetaData(ObjectName, fsID, MetaData);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(ArchiveStreamServiceTest, GetFileMetaDataF)
{
    mp_string ObjectName = "objname";
    mp_string fsID = "fsid";
    mp_string MetaData = "sdkjfosuiodjgbkl";
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Fail);
    int ret = m_archiveSteamService.GetFileMetaData(ObjectName, fsID, MetaData);
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(ArchiveStreamServiceTest, EndRecoverT)
{
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Succ);
    int ret = m_archiveSteamService.EndRecover();
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(ArchiveStreamServiceTest, EndRecoverF)
{
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Fail);
    int ret = m_archiveSteamService.EndRecover();
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(ArchiveStreamServiceTest, ConnectT)
{
    std::string busiIp;
    int busiPort = 1008;
    bool openSsl {false};
    int ret = m_archiveSteamService.Connect(busiIp, busiPort, openSsl);
    EXPECT_EQ(ret, MP_FAILED);
    busiPort = 65539;
    ret = m_archiveSteamService.Connect(busiIp, busiPort, openSsl);
    EXPECT_EQ(ret, MP_FAILED);
    busiIp = "8.40.0.0";
    stub.set(ADDR(ArchiveStreamClientHandler, GetConnectState), Ret_Succ);
    ret = m_archiveSteamService.Connect(busiIp, busiPort, openSsl);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(ArchiveStreamServiceTest, ConnectF)
{
    std::string busiIp;
    int busiPort = 1008;
    bool openSsl {false};
    stub.set(ADDR(ArchiveStreamService, SplitIpList), Ret_False);
    int ret = m_archiveSteamService.Connect(busiIp, busiPort, openSsl);
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(ArchiveStreamServiceTest, SendDppMsgWithResponesT)
{
    std::string taskId = "jsyuihmddusdu527sdf";
    Json::Value strReqMsg;
    ArchiveStreamGetFileRsq strRspMsg;
    unsigned int reqCmd = 72;
    unsigned int reciveCount = 7;
    stub.set(ADDR(ArchiveStreamClientHandler, GetResponseMessage), Ret_True);
    int ret = m_archiveSteamService.SendDppMsgWithRespones(taskId, strReqMsg, strRspMsg, reqCmd, reciveCount);
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(ArchiveStreamServiceTest, SendDppMsgT)
{
    std::string taskId = "jsyuihmddusdu527sdf";
    std::string metaFileDir;
    stub.set(ADDR(ArchiveStreamClientHandler, GetResponseMessage), Ret_True);
    int ret = m_archiveSteamService.PrepareRecovery(metaFileDir);
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(ArchiveStreamServiceTest, MountFileSystemT)
{
    mp_string storeIp = "8.40.11.00";
    mp_string sharePath = "dsdjfdkj";
    mp_string mountPath = "dsdjfdkj";
    int ret = m_archiveSteamService.MountFileSystem(storeIp, sharePath, mountPath);
    EXPECT_EQ(ret, MP_ERROR);
}

TEST_F(ArchiveStreamServiceTest, GetDoradoIpT)
{
    std::vector<mp_string> hostIpv4List;
    int ret = m_archiveSteamService.GetDoradoIp(hostIpv4List);
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(ArchiveStreamServiceTest, CheckIPLinkStatusT)
{
    std::vector<mp_string> hostIpv4List;
    std::vector<mp_string> hostIpv6List;
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Succ);
    int ret = m_archiveSteamService.CheckIPLinkStatus(hostIpv4List, hostIpv6List);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(ArchiveStreamServiceTest, CheckIPLinkStatusF)
{
    std::vector<mp_string> hostIpv4List;
    std::vector<mp_string> hostIpv6List;
    stub.set(ADDR(ArchiveStreamService, SendDppMsg), Ret_Fail);
    int ret = m_archiveSteamService.CheckIPLinkStatus(hostIpv4List, hostIpv6List);
    EXPECT_EQ(ret, MP_FAILED);
}



