#include "message/rest/message_processTest.h"
#include "message/rest/message_process.h"
#include "common/Log.h"
#include "common/ErrorCode.h"

namespace {
mp_void LogTest() {}
#define DoGetJsonStringTest() do { \
    stub11.set(ADDR(CLogger, Log), LogTest); \
} while (0)
}
mp_int32 WriteSS(const mp_string& str)
{
    return -1;
}

mp_void CompleteTest()
{

}
mp_void SendFailedRspTest()
{

}
mp_int32 ReadWaitF(CHttpRequest& req)
{
    return MP_FAILED;
}

mp_int32 ReadWaitS(CHttpRequest& req)
{
    return MP_SUCCESS;
}

mp_uint32 GetLengthTest()
{
    return 2;
}

mp_uint32 GetContentLenTest()
{
    return 2;
}

mp_int32 ReadDataTestF(CHttpRequest& req, CMessage_Block& msg)
{
    return MP_FAILED;
}

mp_int32 ReadDataTest(CHttpRequest& req, CMessage_Block& msg)
{
    return MP_SUCCESS;
}

mp_int32 readStrOverLoadTest(mp_char* b, mp_int32 l)
{
    return ERROR_COMMON_MSG_BLOCK_OVERLOAD;
}

TEST_F(CRequestMsgTest, GetQueryParamTest)
{
    CRequestURL requrl;
    requrl.GetQueryParam();
}

TEST_F(CRequestMsgTest, GetSpecialQueryParamTest)
{
    CRequestURL requrl;
    mp_string strKey;
    requrl.GetSpecialQueryParam(strKey);
}

TEST_F(CRequestMsgTest, SetHttpTypeTest)
{
    CResponseMsg requrl;
    CResponseMsg::enRspType strKey;
    requrl.SetHttpType(strKey);
}

TEST_F(CRequestMsgTest, GetWritePtrTest)
{
    CResponseMsg requrl;
    CResponseMsg::enRspType strKey;
    requrl.SetHttpType(strKey);
    CMessage_Block msg;
    msg.GetWritePtr();
}

TEST_F(CRequestMsgTest, AddLengthTest)
{
    CResponseMsg requrl;
    CResponseMsg::enRspType strKey;
    requrl.SetHttpType(strKey);
    CMessage_Block msg;
    mp_uint32 n;
    msg.AddLength(n);
    n = 1;
    char a[] = "aa";
    msg.m_wirte_ptr = a;
    msg.AddLength(n);
}

TEST_F(CRequestMsgTest, GetLengthTest)
{
    CResponseMsg requrl;
    CResponseMsg::enRspType strKey;
    requrl.SetHttpType(strKey);
    CMessage_Block msg;
    mp_uint32 n;
    msg.GetLength();
}

TEST_F(CRequestMsgTest, GetSizeTest)
{
    CResponseMsg requrl;
    CResponseMsg::enRspType strKey;
    requrl.SetHttpType(strKey);
    CMessage_Block msg;
    mp_uint32 n;
    msg.GetSize();
}

TEST_F(CRequestMsgTest, ResizeTest)
{
    DoGetJsonStringTest();
    CResponseMsg requrl;
    CResponseMsg::enRspType strKey;
    requrl.SetHttpType(strKey);
    CMessage_Block msg;
    mp_uint32 n;
    msg.m_size = 10;
    n = 1;
    msg.Resize(n);
    n = 11;
    msg.Resize(n);
    char *a  = new char[3];
    msg.m_data_block = a;
    msg.Resize(n);
}

TEST_F(CRequestMsgTest, GetReadPtrTest)
{
    DoGetJsonStringTest();
    CResponseMsg requrl;
    CResponseMsg::enRspType strKey;
    requrl.SetHttpType(strKey);
    CMessage_Block msg;
    msg.GetReadPtr();
}

TEST_F(CRequestMsgTest, ParseURLTest)
{
    DoGetJsonStringTest();
    CRequestURL requrl;
    mp_string strKey;
    requrl.ParseURL();
}

TEST_F(CRequestMsgTest, SetQueryParamTest)
{
    DoGetJsonStringTest();
    CRequestURL requrl;
    mp_string strKey;
    mp_string strQueryParam = "aaa";
    mp_bool bUTF8ToANSI = 1;
    requrl.SetQueryParam(strQueryParam, bUTF8ToANSI);
}

TEST_F(CRequestMsgTest, GetServiceNameTest)
{
    DoGetJsonStringTest();
    CRequestURL requrl;
    mp_string strKey;
    requrl.GetServiceName();
    requrl.m_procURL = "sadas/dsa";
    requrl.GetServiceName();
}


/*
 * 用例名称：支持/v1/agent格式url解析
 * 前置条件：1.Mock log 2.m_procURL have /v1
 * check点：返回agent后的servciename
 */
TEST_F(CRequestMsgTest, GetServiceNameWithV1Test)
{
    DoGetJsonStringTest();
    CRequestURL requrl;
    requrl.m_procURL = "/v1/service";
    mp_string serviceName = requrl.GetServiceName();
    EXPECT_EQ(serviceName, "service");
    EXPECT_EQ(requrl.m_version, "v1");
}

/*
 * 用例名称：支持agent格式url解析
 * 前置条件：1.Mock log 2.m_procURL have no /v1
 * check点：返回agent后的servciename
 */
TEST_F(CRequestMsgTest, GetServiceNameNoWithV1Test)
{
    DoGetJsonStringTest();
    CRequestURL requrl;
    requrl.m_procURL = "/service";
    mp_string serviceName = requrl.GetServiceName();
    EXPECT_EQ(serviceName, "service");
    EXPECT_EQ(requrl.m_version, "");
}

/*
 * 用例名称：根据原始URL解析需要的URL，
 *      通过/agent/service/aa 解析为/service/aaa
 *      通过/v1/agent/service/aa 解析为/v1/service/aaa
 * 前置条件：1.Mock log
 * check点：check解析后的URL连接
 */
TEST_F(CRequestMsgTest, ParseURLWithNewFormatTest)
{
    DoGetJsonStringTest();
    CRequestURL requrl;
    requrl.m_oriURL = "/agent/service/aa";
    requrl.ParseURL();
    EXPECT_EQ(requrl.m_procURL, "/service/aa");

    requrl.m_oriURL = "/v1/agent/service/aa";
    requrl.ParseURL();
    EXPECT_EQ(requrl.m_procURL, "/v1/service/aa");
}

TEST_F(CRequestMsgTest, GetCutURLTest)
{
    DoGetJsonStringTest();
    CRequestURL requrl;
    mp_string strKey;
    mp_int32 indexValue = 2;
    requrl.m_procURL = "";
    requrl.GetCutURL(indexValue);
    requrl.m_procURL = "aaa/bbb/aa";
    requrl.GetCutURL(indexValue);
}

TEST_F(CRequestMsgTest, ReadWaitTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsgBody requrl;
    mp_string strKey;
    mp_int32 indexValue = 2;
    CHttpRequest req;
    stub.set(ADDR(CRequestMsgBody, ReadData), ReadDataTestF);
    requrl.ReadWait(req);
    stub.reset(ADDR(CRequestMsgBody, ReadData));
    stub.set(ADDR(CRequestMsgBody, ReadData), ReadDataTest);
    stub.set(ADDR(CMessage_Block, GetLength), GetLengthTest);
    stub.set(ADDR(CHttpRequest, GetContentLen), GetContentLenTest);
    requrl.ReadWait(req);
    stub.reset(ADDR(CRequestMsgBody, ReadData));
    stub.reset(ADDR(CMessage_Block, GetLength));
    stub.reset(ADDR(CHttpRequest, GetContentLen));
}

TEST_F(CRequestMsgTest, ReadDataTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsgBody *requrl = new CRequestMsgBody();
    mp_string strKey;
    mp_int32 indexValue = 2;
    CHttpRequest *req = new CHttpRequest();
    std::auto_ptr<CMessage_Block> msg(new CMessage_Block());
    // requrl->ReadData(*req, msg);
}

/*
*用例名称：从HTTP流汇中读取的消息块长度超限
*前置条件：读取信息块超过1Mb 
*check点：返回消息块长度超上限的特定错误码
*/
TEST_F(CRequestMsgTest, ReadData_OverLoad_Test)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsgBody *requrl = new CRequestMsgBody();
    mp_string strKey;
    mp_int32 indexValue = 2;
    CHttpRequest *req = new CHttpRequest();
    CMessage_Block *msg = new CMessage_Block();
    stub.set(ADDR(CHttpRequest, ReadStr), readStrOverLoadTest);
    msg->m_size = 3;
    msg->m_length = 1;
    EXPECT_EQ(ERROR_COMMON_MSG_BLOCK_OVERLOAD, requrl->ReadData(*req, *msg));
    delete req;
    delete msg;
}

TEST_F(CRequestMsgTest, ParseJsonTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsgBody requrl;
    mp_string strKey;
    mp_int32 indexValue = 2;
    CHttpRequest req;
    std::auto_ptr<CMessage_Block> msg(new CMessage_Block());
    // requrl.ParseJson(msg);
}

TEST_F(CRequestMsgTest, JsonValueToStringTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsgBody requrl;
    mp_string strKey;
    mp_int32 indexValue = 2;
    CHttpRequest req;
    Json::Value v;
    v["aa"] = "aaaaaaaa";
    requrl.JsonValueToString(v);
}

TEST_F(CRequestMsgTest, GetValueTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsgBody requrl;
    mp_string strKey;
    mp_int32 indexValue = 2;
    CHttpRequest req;
    mp_string value;
    requrl.GetValue("aa", value);
    
    requrl.m_msgJsonData["aa"] = "aaa";
    requrl.GetValue("aa", value);
}

TEST_F(CRequestMsgTest, GetOriMsgTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsgBody requrl;
    mp_string strKey;
    mp_int32 indexValue = 2;
    CHttpRequest req;
    mp_char buf;
    mp_uint32 len;
    // requrl.m_raw_msg.get() = nullptr;
    requrl.GetOriMsg(buf, len);
    requrl.m_raw_msg.reset(new CMessage_Block());

}

TEST_F(CRequestMsgTest, ParseTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg requrl;
    stub.set(ADDR(CRequestMsgBody, ReadWait), ReadWaitF);
    requrl.Parse();

    stub.set(ADDR(CRequestMsgBody, ReadWait), ReadWaitS);
    requrl.Parse();
    stub.reset(ADDR(CRequestMsgBody, ReadWait));
}

TEST_F(CRequestMsgTest, SendTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CResponseMsg requrl;
    stub.set(ADDR(CResponseMsg, SendJson), ReadWaitS);
    requrl.m_httpType = CResponseMsg::enRspType::RSP_JSON_TYPE;
    EXPECT_EQ(MP_SUCCESS, requrl.Send());
    stub.reset(ADDR(CResponseMsg, SendJson));

    stub.set(ADDR(CResponseMsg, SendAttchment), ReadWaitS);
    requrl.m_httpType = CResponseMsg::enRspType::RSP_ATTACHMENT_TYPE;
    EXPECT_EQ(MP_SUCCESS, requrl.Send());
    stub.reset(ADDR(CResponseMsg, SendAttchment));

    requrl.m_httpType = CResponseMsg::enRspType(10);
    EXPECT_EQ(MP_FAILED, requrl.Send());
}

TEST_F(CRequestMsgTest, SendJsonTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CResponseMsg requrl;
    stub.set(ADDR(CResponseMsg, SendJson), ReadWaitS);
    requrl.m_httpType = CResponseMsg::enRspType::RSP_JSON_TYPE;
    requrl.SendJson();
    stub.reset(ADDR(CResponseMsg, SendJson));
}

TEST_F(CRequestMsgTest, SendAttchmentTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CResponseMsg requrl;
    stub.set(ADDR(CResponseMsg, SendFailedRsp), SendFailedRspTest);
    stub.set(ADDR(CHttpResponse, Complete), CompleteTest);
    // requrl.SendAttchment();
    stub.reset(ADDR(CResponseMsg, SendFailedRsp));
    stub.reset(ADDR(CHttpResponse, Complete));
}

TEST_F(CRequestMsgTest, SetHeaderTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CResponseMsg requrl;
    mp_string strAttachFileName;
    requrl.SetHeader(strAttachFileName);
}

TEST_F(CRequestMsgTest, PackageReponseTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CResponseMsg requrl;
    Json::Value strAttachFileName;
    requrl.PackageReponse(strAttachFileName);
}

TEST_F(CRequestMsgTest, SendFailedRspTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CResponseMsg requrl;
    stub.set(ADDR(CHttpResponse, WriteS), WriteSS);
    requrl.SendFailedRsp();
}

TEST_F(CRequestMsgTest, GetNumTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CUrlUtils requrl;
    mp_char s[] = "aa";
    mp_uint32 len = 2;
    mp_uint32 b;
    mp_uint64 ret;
    requrl.GetNum(s, len, b ,ret);
}
