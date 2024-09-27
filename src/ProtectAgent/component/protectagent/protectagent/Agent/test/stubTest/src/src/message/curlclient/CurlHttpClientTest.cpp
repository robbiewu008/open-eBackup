#include <vector>
#include "message/curlclient/CurlHttpClientTest.h"
#include "message/curlclient/CurlHttpClient.h"
#include "message/curlclient/SSLHandle.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"
#include "common/Path.h"

namespace {
mp_void LogTest()
{}

static const int MP_SUCCESS_NO_IMPORTED = -1;
static const int MP_SUCCESS_IS_IMPORTED = 0;

#define DoLogTest()                                                                                                    \
    do {                                                                                                               \
        stub.set(ADDR(CLogger, Log), LogTest);                                                                         \
    } while (0)

CURL* StubCurlEasyInit()
{
    return nullptr;
}

mp_void StubVoid()
{
    return;
}

mp_uint32 StubCurlEasyPerform()
{
    return CURLE_OK;
}

IHttpClient* StubCurlHttpClient()
{
    return nullptr;
}

mp_int32 StubFailed(mp_void* pobj)
{
    return MP_FAILED;
}

mp_int32 StubSuccess(mp_void* pobj)
{
    return MP_SUCCESS;
}

curl_slist* StubCurlSlistAppend()
{
    curl_slist* curl = new curl_slist;
    return curl;
}

char* StubcurlEasyStrErrorNull()
{
    char* errDes = nullptr;
    return errDes;
}

char* StubCurlEasyStrError()
{
    char* errDes = "test";
    return errDes;
}

mp_bool StubFalse(mp_void* pobj)
{
    return MP_FALSE;
}
mp_int32 StubGetGeneralThumbprint(
    const mp_string& url, mp_string& thumbPrint, const mp_string& algorithm, const mp_uint32 time_out)
{
    return -1;
}

mp_string GetCertificatePositionStart(const mp_string& url, const mp_uint32 time_out)
{
    return "Ce";
}

mp_string GetCertificatePositionEnd(const mp_string& url, const mp_uint32 time_out)
{
    return "Cert:";
}

mp_string GetCertificateLt(const mp_string& url, const mp_uint32 time_out)
{
    return "-----END CERTIFICATE-----Cert:";
}

mp_string GetCertificate(const mp_string& url, const mp_uint32 time_out)
{
    return "Cert:l-----END CERTIFICATE-----";
}
}  // namespace

mp_int32 StubGetValueInt32AndValueIs1(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    iValue = 1;
    return MP_SUCCESS;
}

mp_int32 StubGetValueInt32AndValueIs0(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    iValue = 0;
    return MP_SUCCESS;
}

mp_int32 StubGetValueStringPram4SuccessTest(
    const mp_string& strParentSection, const mp_string& strChildSection, const mp_string&  strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueStringPram4FailTest(
    const mp_string& strParentSection, const mp_string& strChildSection, const mp_string&  strKey, mp_string& strValue)
{
    return MP_FAILED;
}

mp_int32 StubGetValueStringSuccessTest(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueStringFailTest(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_FAILED;
}

mp_void StubDecryptStr(const mp_string& inStr, mp_string& outStr)
{
    outStr = "test";
    return;
}

mp_void StubDecryptStrEmpty(const mp_string& inStr, mp_string& outStr)
{
    return;
}

mp_int32 StubSetCertInCurlSUCCESS()
{
    return MP_SUCCESS;
}

mp_int32 StubSetCertInCurlFAILED()
{
    return MP_FAILED;
}

mp_int32 StubCheckCRLIMPORTED()
{
    return MP_SUCCESS_IS_IMPORTED;
}

mp_int32 StubCheckCRLNOIMPORTED()
{
    return MP_SUCCESS_NO_IMPORTED;
}

mp_int32 StubCheckCRLNOEXISTS()
{
    return MP_NOEXISTS;
}

mp_int32 StubCheckCRLERROR()
{
    return MP_ERROR;
}

mp_string StubGetConfFilePath(const mp_string& strFileName)
{
    return "test";
}

mp_int32 StubReadFileSUCCESSAndValueIs0(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.clear();
    vecOutput.push_back("CERTIFICATE_REVOCATION=0");
    return MP_SUCCESS;
}

mp_int32 StubReadFileSUCCESSAndValueIs1(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.clear();
    vecOutput.push_back("CERTIFICATE_REVOCATION=1");
    return MP_SUCCESS;
}

mp_int32 StubReadFileSUCCESSAndValueAbnormal(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.clear();
    vecOutput.push_back("CERTIFICATE_REVOCATION=");
    return MP_SUCCESS;
}

mp_int32 StubReadFileSUCCESSAndKeyNoExist(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.clear();
    vecOutput.push_back("test=0");
    return MP_SUCCESS;
}

mp_int32 StubReadFileFAILED(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    return MP_FAILED;
}

/*
 * 用例名称：初始化
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, Init)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    mp_bool iRet;
    stub.set(curl_easy_init, StubCurlEasyInit);

    iRet = curlHttpResponse.Init();
    EXPECT_EQ(MP_FALSE, iRet);
    stub.reset(curl_easy_init);
}

/*
 * 用例名称：配置双向证书身份验证
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, ConfigureTwoWayCertsAuthenticate)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    HttpRequest req;
    mp_int32 iRet;

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetValueInt32AndValueIs0);
    iRet = curlHttpResponse.ConfigureTwoWayCertsAuthenticate(req);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetValueInt32AndValueIs1);
    stub.set(ADDR(CurlHttpResponse, SetCertInCurl), StubSetCertInCurlFAILED);
    iRet = curlHttpResponse.ConfigureTwoWayCertsAuthenticate(req);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(CurlHttpResponse, SetCertInCurl), StubSetCertInCurlSUCCESS);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(
        CConfigXmlParser, GetValueString), StubGetValueStringPram4FailTest);
    iRet = curlHttpResponse.ConfigureTwoWayCertsAuthenticate(req);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, const mp_string&, mp_string&))ADDR(
        CConfigXmlParser, GetValueString), StubGetValueStringPram4SuccessTest);
    stub.set(DecryptStr, StubDecryptStrEmpty);
    iRet = curlHttpResponse.ConfigureTwoWayCertsAuthenticate(req);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(DecryptStr, StubDecryptStr);
    iRet = curlHttpResponse.ConfigureTwoWayCertsAuthenticate(req);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 用例名称：设置CURL工具安全配置
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, SetCertInCurl)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    HttpRequest req;
    mp_int32 iRet;

    req.caInfo = "";
    req.sslKey = "test";
    req.sslCert = "test";
    iRet = curlHttpResponse.SetCertInCurl(req);
    EXPECT_EQ(NOT_SET_ALL_TWO_WAY_CERTS, iRet);

    req.caInfo = "test";
    req.sslKey = "";
    req.sslCert = "test";
    iRet = curlHttpResponse.SetCertInCurl(req);
    EXPECT_EQ(NOT_SET_ALL_TWO_WAY_CERTS, iRet);

    req.caInfo = "test";
    req.sslKey = "test";
    req.sslCert = "";
    iRet = curlHttpResponse.SetCertInCurl(req);
    EXPECT_EQ(NOT_SET_ALL_TWO_WAY_CERTS, iRet);

    req.caInfo = "test";
    req.sslKey = "test";
    req.sslCert = "test";
    stub.set(ADDR(CurlHttpResponse, CheckCRLStatus), StubCheckCRLIMPORTED);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringFailTest);
    iRet = curlHttpResponse.SetCertInCurl(req);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringSuccessTest);
    iRet = curlHttpResponse.SetCertInCurl(req);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(CurlHttpResponse, CheckCRLStatus), StubCheckCRLNOIMPORTED);
    iRet = curlHttpResponse.SetCertInCurl(req);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(CurlHttpResponse, CheckCRLStatus), StubCheckCRLNOEXISTS);
    iRet = curlHttpResponse.SetCertInCurl(req);
    EXPECT_EQ(MP_ERROR, iRet);

    stub.set(ADDR(CurlHttpResponse, CheckCRLStatus), StubCheckCRLERROR);
    iRet = curlHttpResponse.SetCertInCurl(req);
    EXPECT_EQ(MP_ERROR, iRet);
}

/*
 * 用例名称：识别吊销证书导入标示
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, CheckCRLStatus)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    HttpRequest req;
    mp_int32 iRet;
    std::vector<mp_string> vecOutput;

    stub.set(ADDR(CPath, GetConfFilePath), StubGetConfFilePath);
    stub.set(ADDR(CMpFile, ReadFile), StubReadFileFAILED);
    iRet = curlHttpResponse.CheckCRLStatus();
    EXPECT_EQ(MP_ERROR, iRet);

    stub.set(ADDR(CMpFile, ReadFile), StubReadFileSUCCESSAndKeyNoExist);
    iRet = curlHttpResponse.CheckCRLStatus();
    EXPECT_EQ(MP_ERROR, iRet);

    stub.set(ADDR(CMpFile, ReadFile), StubReadFileSUCCESSAndValueIs1);
    iRet = curlHttpResponse.CheckCRLStatus();
    EXPECT_EQ(MP_SUCCESS_IS_IMPORTED, iRet);

    stub.set(ADDR(CMpFile, ReadFile), StubReadFileSUCCESSAndValueIs0);
    iRet = curlHttpResponse.CheckCRLStatus();
    EXPECT_EQ(MP_SUCCESS_NO_IMPORTED, iRet);

    stub.set(ADDR(CMpFile, ReadFile), StubReadFileSUCCESSAndValueAbnormal);
    iRet = curlHttpResponse.CheckCRLStatus();
    EXPECT_EQ(MP_ERROR, iRet);
}

/*
 * 用例名称：发送请求
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, SendRequest)
{
    DoLogTest();
    mp_uint32 time_out;
    HttpRequest req;
    mp_int32 iRet;
    CurlHttpResponse curlHttpResponse;

    req.specialNetworkCard = "test";
    stub.set(&CurlHttpResponse::ConfigureTwoWayCertsAuthenticate, StubFailed);
    curlHttpResponse.SendRequest(req, time_out);

    req.hostinfo = "test hostinfo";
    req.url = "https://test.com";
    req.sslKey = "";
    req.sslCert = "";
    stub.set(&CurlHttpResponse::ConfigureTwoWayCertsAuthenticate, StubSuccess);
    curlHttpResponse.SendRequest(req, time_out);
    stub.reset(&CurlHttpResponse::ConfigureTwoWayCertsAuthenticate);
    stub.reset(curl_slist_append);
}

/*
 * 用例名称：是否成功判断
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, Success)
{
    DoLogTest();
    mp_bool iRet;
    CurlHttpResponse curlHttpResponse;
    curlHttpResponse.m_StatusCode = SC_CREATED;
    curlHttpResponse.m_ErrorCode = CURLE_OK;

    iRet = curlHttpResponse.Success();
    EXPECT_EQ(iRet, MP_TRUE);
}

/*
 * 用例名称：是否忙碌
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, Busy)
{
    DoLogTest();
    mp_bool iRet;
    CurlHttpResponse curlHttpResponse;
    curlHttpResponse.m_StatusCode = SC_SERVICE_UNAVAILABLE;

    iRet = curlHttpResponse.Busy();
    EXPECT_EQ(iRet, MP_TRUE);
}

/*
 * 用例名称：设置rest请求方式
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, SetMethod)
{
    DoLogTest();
    mp_string method;
    CurlHttpResponse curlHttpResponse;

    method = "POST";
    curlHttpResponse.SetMethod(method);

    method = "GET";
    curlHttpResponse.SetMethod(method);

    method = "PUT";
    curlHttpResponse.SetMethod(method);
}

/*
 * 用例名称：设置请求头
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, SetHeaders)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    std::set<std::pair<mp_string, mp_string>> heads;
    heads.insert(std::pair<mp_string, mp_string>("token", "dhakjfajfjldjaijfajljfla"));

    curlHttpResponse.SetHeaders(heads);
    heads.clear();
}

/*
 * 用例名称：设置Cert
 * 前置条件：无
 * check点：无
 */
TEST_F(CurlHttpClientTest, SetCert)
{
    DoLogTest();
    mp_string cert;
    CurlHttpResponse curlHttpResponse;

    curlHttpResponse.SetCert(cert);
    cert = "test";
    curlHttpResponse.SetCert(cert);
}

/*
 * 用例名称：获取数据回调
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, GetDataCallback)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    void* ptr = NULL;
    size_t size;
    size_t counts;
    void* self = NULL;
    auto iRet = curlHttpResponse.GetDataCallback(ptr, size, counts, self);
    EXPECT_EQ(iRet, 0);

    ptr = new (char);
    self = new (CurlHttpResponse);
    iRet = curlHttpResponse.GetDataCallback(ptr, size, counts, self);
    EXPECT_EQ(iRet, 0);
}

/*
 * 用例名称：获取头回调
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, GetHeaderCallback)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    void* ptr = NULL;
    size_t size;
    size_t counts;
    void* self = NULL;
    auto iRet = curlHttpResponse.GetHeaderCallback(ptr, size, counts, self);
    EXPECT_EQ(iRet, 0);

    ptr = new (char);
    self = new (CurlHttpResponse);
    iRet = curlHttpResponse.GetHeaderCallback(ptr, size, counts, self);
    EXPECT_EQ(iRet, 0);
}

/*
 * 用例名称：解析状态行
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, ParseStatusLine)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    mp_string status_line = "test test test";
    curlHttpResponse.ParseStatusLine(status_line);
}

/*
 * 用例名称：接收消息头
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, RecieveHeader)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    mp_string header = "test:test";
    curlHttpResponse.RecieveHeader(header);
    EXPECT_EQ(curlHttpResponse.m_Headers.size(), 1);
}

/*
 * 用例名称：获取状态描述
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, GetHttpStatusDescribe)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    mp_string sRet = curlHttpResponse.GetHttpStatusDescribe();
    EXPECT_EQ(sRet, curlHttpResponse.m_StatusDescribe);
}

/*
 * 用例名称：获取信息
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, GetErrString)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;

    stub.set(curl_easy_strerror, StubcurlEasyStrErrorNull);
    mp_string sRet = curlHttpResponse.GetErrString();
    EXPECT_EQ(sRet, "Unknow error!");

    stub.set(curl_easy_strerror, StubCurlEasyStrError);
    sRet = curlHttpResponse.GetErrString();
    EXPECT_EQ(sRet, "test");
    stub.reset(curl_easy_strerror);
}

/*
 * 用例名称：获取头信息
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, GetHeadByName)
{
    DoLogTest();
    mp_string header_name;
    CurlHttpResponse curlHttpResponse;
    std::set<mp_string> testSet;
    curlHttpResponse.m_Headers.insert(std::pair<mp_string, std::set<mp_string>>("test", testSet));

    header_name = "test";
    curlHttpResponse.GetHeadByName(header_name);

    header_name = "test2";
    curlHttpResponse.GetHeadByName(header_name);
}

/*
 * 用例名称：获取Cookies
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, GetCookies)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    curlHttpResponse.GetCookies();
}

/*
 * 用例名称：获取返回头内容
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, GetHeaders)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    curlHttpResponse.GetHeaders();
}

/*
 * 用例名称：发送请求
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, SendRequestTest)
{
    DoLogTest();
    mp_uint32 time_out;
    HttpRequest req;
    mp_int32 iRet;
    CurlHttpClient curlHttpClient;

    stub.set(&CurlHttpResponse::Init, StubFalse);
    IHttpResponse* resp = curlHttpClient.SendRequest(req, time_out);
    EXPECT_EQ(resp, nullptr);
}

/*
 * 用例名称：创建Client
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, CreateClient)
{
    DoLogTest();
    CurlHttpClient curlHttpClient;

    IHttpClient* pClient = curlHttpClient.CreateClient();
    EXPECT_NE(pClient, nullptr);
}

/*
 * 用例名称：测试连通性
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, TestConnect)
{
    DoLogTest();
    CurlHttpClient curlHttpClient;
    mp_string url = "";
    mp_uint32 time_out = 200;
    mp_bool ret = curlHttpClient.TestConnect(url, time_out);
    EXPECT_EQ(ret, MP_FALSE);

    stub.set(curl_easy_init, StubCurlEasyInit);
    ret = curlHttpClient.TestConnect(url, time_out);
    EXPECT_EQ(ret, MP_FALSE);
    stub.reset(curl_easy_init);

    url = "https://test.com";
    stub.set(curl_easy_perform, StubCurlEasyPerform);
    ret = curlHttpClient.TestConnect(url, time_out);
    EXPECT_EQ(ret, MP_TRUE);
    stub.reset(curl_easy_init);
    stub.reset(curl_easy_perform);
}

/*
 * 用例名称：获取证书
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, GetCertificate)
{
    DoLogTest();
    CurlHttpClient curlHttpClient;
    mp_string url;
    mp_uint32 time_out;

    stub.set(curl_easy_init, StubCurlEasyInit);
    mp_string ret = curlHttpClient.GetCertificate(url, time_out);
    EXPECT_EQ(ret, "");
    stub.reset(curl_easy_init);

    stub.set(curl_easy_perform, StubCurlEasyPerform);
    ret = curlHttpClient.GetCertificate(url, time_out);
    EXPECT_EQ(ret, "");
    stub.reset(curl_easy_perform);
    stub.reset(curl_easy_getinfo);
}

/*
 * 用例名称：获取密钥
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, GetThumbPrint)
{
    DoLogTest();
    CurlHttpClient curlHttpClient;
    mp_string url;
    mp_string thumbPrint;
    mp_uint32 time_out;

    stub.set(ADDR(CurlHttpClient, GetGeneralThumbprint), StubGetGeneralThumbprint);
    mp_int32 iRet = curlHttpClient.GetThumbPrint(url, thumbPrint, time_out);
    EXPECT_EQ(iRet, -1);
    stub.reset(&CurlHttpClient::GetGeneralThumbprint);
}

/*
 * 用例名称：获取密钥
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, GetGeneralThumbprint)
{
    DoLogTest();
    CurlHttpClient curlHttpClient;
    mp_string url;
    mp_string thumbPrint;
    mp_string algorithm;
    mp_uint32 time_out;

    stub.set(GetCertificate, GetCertificatePositionStart);
    mp_int32 iRet = curlHttpClient.GetGeneralThumbprint(url, thumbPrint, algorithm, time_out);
    EXPECT_EQ(iRet, -1);

    stub.set(GetCertificate, GetCertificatePositionEnd);
    iRet = curlHttpClient.GetGeneralThumbprint(url, thumbPrint, algorithm, time_out);
    EXPECT_EQ(iRet, -1);

    stub.set(GetCertificate, GetCertificateLt);
    iRet = curlHttpClient.GetGeneralThumbprint(url, thumbPrint, algorithm, time_out);
    EXPECT_EQ(iRet, -1);

    stub.reset(ADDR(CurlHttpClient, GetCertificate));
}

/*
 * 用例名称：设置数据回调
 * 前置条件：文件名不为空
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, SetCallBackTest)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    HttpRequest req;
    req.fileName = "";
    FILE* fp = NULL;
    mp_int32 iRet = curlHttpResponse.SetCallBack(req, &fp);
    EXPECT_EQ(iRet, 0);

    req.fileName = "/DataProtect_client.zip";
    iRet = curlHttpResponse.SetCallBack(req, &fp);
    std::cout<<"ret: " <<(fp != NULL) <<"\n";
    fclose(fp);
    EXPECT_EQ(iRet, 0);
}

/*
 * 用例名称：清理curl
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CurlHttpClientTest, CleanCurlTest)
{
    DoLogTest();
    CurlHttpResponse curlHttpResponse;
    HttpRequest req;
    FILE* fp = NULL;
    req.fileName = "/DataProtect_client.zip";
    mp_int32 iRet = curlHttpResponse.SetCallBack(req, &fp);
    std::cout<<(fp != NULL)<<"\n";
    EXPECT_EQ(1, (fp != NULL));

    curl_slist* headers = NULL;
    struct curl_slist* host = NULL;
    curlHttpResponse.CleanCurl(headers, host, fp);
    EXPECT_EQ(1, (fp != NULL));
}
