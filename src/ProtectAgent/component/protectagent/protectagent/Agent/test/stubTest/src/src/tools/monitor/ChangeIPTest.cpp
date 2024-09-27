#include "tools/agentcli/ChangeIPTest.h"
#include "message/tcp/CSocket.h"
#include "tools/agentcli/RegisterHost.h"
using namespace std;

namespace {
mp_int32 CheckHostLinkStatusFailed(
    const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_FAILED;
}

mp_int32 CheckHostLinkStatusSucc(
    const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_SUCCESS;
}

uid_t getuidZero()
{
    return 0;
}

uid_t getuidNotZero()
{
    return 1;
}

mp_bool CheckIsIPv6OrIPv4False(void* pThis, const std::string& ip)
{
    return MP_FALSE;
}

mp_bool CheckIsIPv6OrIPv4True(void* pThis, const std::string& ip)
{
    return MP_TRUE;
}

mp_void GetInputNotAnyIP(const mp_string& strHint, mp_string& strInput, mp_int32 iInputLen)
{
    strInput = "1.1.1.1";
}

mp_bool IsLocalIPFalse(mp_string strIP)
{
    return MP_FALSE;
}

mp_bool IsLocalIPTrue(mp_string strIP)
{
    return MP_TRUE;
}

mp_int32 GetIPAddressFailed(mp_string& strIP)
{
    return MP_FAILED;
}

mp_int32 GetIPAddressSucc(mp_string& strIP)
{
    return MP_SUCCESS;
}

mp_int32 strcmpZero(const char *__s1, const char *__s2)
{
    return 0;
}

mp_int32 strcmpNotZero(const char *__s1, const char *__s2)
{
    return 1;
}

mp_int32 GetPMIPandPortFailed()
{
    return MP_FAILED;
}

mp_int32 GetPMIPandPortSucc()
{
    return MP_SUCCESS;
}

mp_int32 RegisterHost2PMFailed()
{
    return MP_FAILED;
}

mp_int32 RegisterHost2PMSucc()
{
    return MP_SUCCESS;
}

mp_int32 SetIPAddressFailed(void* pThis, const mp_string& strIP)
{
    return MP_FAILED;
}

mp_int32 SetIPAddressSucc(void* pThis, const mp_string& strIP)
{
    return MP_SUCCESS;
}

mp_void GetInputN(const mp_string& strHint, mp_string& strInput, mp_int32 iInputLen)
{
    strInput = "N";
}

mp_void GetInputY(const mp_string& strHint, mp_string& strInput, mp_int32 iInputLen)
{
    strInput = "Y";
}

mp_int32 RestartNginxFailed()
{
    return MP_FAILED;
}

mp_int32 RestartNginxSucc()
{
    return MP_SUCCESS;
}

mp_int32 GetListenIPAndPortFailed(mp_string& strIP, mp_string& strPort)
{
    return MP_FAILED;
}

mp_int32 GetListenIPAndPortSucc(mp_string& strIP, mp_string& strPort)
{
    return MP_SUCCESS;
}

mp_bool FileExistFalse(const mp_string& pszFilePath)
{
    return MP_FALSE;
}

mp_bool FileExistTrue(const mp_string& pszFilePath)
{
    return MP_TRUE;
}

mp_int32 ReadFileFailed(const mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    return MP_FAILED;
}

mp_int32 ReadFileSucc(mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    vecOutput.push_back("Agent");
    return MP_SUCCESS;
}

mp_int32 WriteFileFailed(mp_string& strFilePath, vector<mp_string>& vecInput)
{
    return MP_FAILED;
}

mp_int32 WriteFileSucc(mp_string& strFilePath, vector<mp_string>& vecInput)
{
    return MP_SUCCESS;
}


}

TEST_F(TestHostTest, HandleTest)
{
    Stub stub;
    mp_string hostIp = "127.0.0.1";
    mp_string hostPort = "53";
    mp_string timeOut = "100";
    TestHost testHost;

    stub.set(ADDR(CSocket, CheckHostLinkStatus), CheckHostLinkStatusFailed);
    mp_int32 iRet = testHost.Handle(hostIp, hostPort, timeOut);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CSocket, CheckHostLinkStatus), CheckHostLinkStatusSucc);
    iRet = testHost.Handle(hostIp, hostPort, timeOut);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CChangeIpTest, HandleTest)
{
    Stub stub;
    ChangeIP changeIP;
    
    stub.set(getuid, getuidZero);
    mp_int32 iRet = changeIP.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(getuid, getuidNotZero);
    stub.set(ADDR(CIP, CheckIsIPv6OrIPv4), CheckIsIPv6OrIPv4False);
    stub.set(ADDR(CPassword, GetInput), GetInputNotAnyIP);
    iRet = changeIP.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CIP, CheckIsIPv6OrIPv4), CheckIsIPv6OrIPv4True);
    stub.set(ADDR(ChangeIP, IsLocalIP), CheckIsIPv6OrIPv4False);
    iRet = changeIP.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ChangeIP, IsLocalIP), CheckIsIPv6OrIPv4True);
    stub.set(ADDR(ChangeIP, GetIPAddress), GetIPAddressFailed);
    iRet = changeIP.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ChangeIP, GetIPAddress), GetIPAddressSucc);
    stub.set(strcmp, strcmpZero);
    iRet = changeIP.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(strcmp, strcmpNotZero);
    stub.set(ADDR(RegisterHost, GetPMIPandPort), GetPMIPandPortFailed);
    iRet = changeIP.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegisterHost, GetPMIPandPort), GetPMIPandPortSucc);
    stub.set(ADDR(RegisterHost, RegisterHost2PM), RegisterHost2PMFailed);
    iRet = changeIP.Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(RegisterHost, RegisterHost2PM), RegisterHost2PMSucc);
    iRet = changeIP.Handle();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CChangeIpTest,ChangeIPAfterTest)
{
    Stub stub;
    ChangeIP changeIP;
    mp_string strInput;

    stub.set(ADDR(ChangeIP, SetIPAddress), SetIPAddressFailed);
    mp_int32 iRet = changeIP.ChangeIPAfter(strInput);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ChangeIP, SetIPAddress), SetIPAddressSucc);
    stub.set(ADDR(CPassword, GetInput), GetInputN);
    iRet = changeIP.ChangeIPAfter(strInput);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(ADDR(CPassword, GetInput), GetInputY);
    stub.set(ADDR(ChangeIP, RestartNginx), RestartNginxFailed);
    iRet = changeIP.ChangeIPAfter(strInput);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ChangeIP, RestartNginx), RestartNginxSucc);
    iRet = changeIP.ChangeIPAfter(strInput);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CChangeIpTest, GetIPAddressTest)
{
    Stub stub;
    ChangeIP changeIP;
    mp_string strIP;

    stub.set(ADDR(CIP, GetListenIPAndPort), GetListenIPAndPortFailed);
    mp_int32 iRet = changeIP.GetIPAddress(strIP);
    EXPECT_EQ(iRet, MP_FAILED);
    
    stub.set(ADDR(CIP, GetListenIPAndPort), GetListenIPAndPortSucc);
    iRet = changeIP.GetIPAddress(strIP);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CChangeIpTest, SetIPAddressTest)
{
    Stub stub;
    ChangeIP changeIP;
    mp_string strIP;

    stub.set(ADDR(CMpFile, FileExist), FileExistFalse);
    mp_int32 iRet = changeIP.GetIPAddress(strIP);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CMpFile, FileExist), FileExistTrue);
    stub.set(ADDR(CMpFile, ReadFile), ReadFileFailed);
    iRet = changeIP.GetIPAddress(strIP);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(CChangeIpTest, GetLocalIPs)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecIPs;   
    ChangeIP IPObj;
    
    iRet = IPObj.GetLocalIPs(vecIPs);
    EXPECT_EQ(MP_SUCCESS, iRet);

    return;
}

TEST_F(CChangeIpTest, IsLocalIP)
{
    mp_bool bRet = MP_FALSE;
    mp_string strIP = "192.168.100.127";
    ChangeIP IPObj;

    Stub stubSign;
    stubSign.set(&ChangeIP::GetLocalIPs, stub_return_ret);

    bRet = IPObj.IsLocalIP(strIP);
    EXPECT_EQ(MP_FALSE, bRet);

    bRet = IPObj.IsLocalIP(strIP);
    EXPECT_EQ(MP_FALSE, bRet);

    return;
}

TEST_F(CChangeIpTest, RestartNginx)
{
    mp_int32 iRet = 0;
    ChangeIP IPObj;
    
    /* ºÏ≤‚ ß∞‹; */
    iRet = IPObj.RestartNginx();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = IPObj.RestartNginx();
    EXPECT_EQ(MP_SUCCESS, iRet);

    return;
}



