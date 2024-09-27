#include "tools/agentcli/ChgNgxPwdTest.h"
#include "common/Utils.h"
#include "securecom/CryptAlg.h"
using namespace std;

namespace {
mp_void LogReturn(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...)
{
    return;
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

mp_int32 ReadFileSucc(const mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    vecOutput.push_back("Agent");
    return MP_SUCCESS;
}

mp_int32 CheckUserPwdFailed()
{
    return MP_FAILED;
}

mp_int32 CheckUserPwdSucc()
{
    return MP_SUCCESS;
}

mp_int32 GetHostSNNumFailed(mp_string& strInput)
{
    return MP_FAILED;
}

mp_int32 GetHostSNNumSucc(mp_string& strInput)
{
    return MP_SUCCESS;
}

mp_int32 ModifyHostSNFailed(vector<mp_string>& vecResult, mp_string& strInput)
{
    return MP_FAILED;
}

mp_int32 ModifyHostSNSucc(vector<mp_string>& vecResult, mp_string& strInput)
{
    return MP_SUCCESS;
}

mp_int32 ChownHostSnFailed(mp_string& strInput)
{
    return MP_FAILED;
}

mp_int32 ChownHostSnSucc(mp_string& strInput)
{
    return MP_SUCCESS;
}

mp_void GetInputEmpty(const mp_string& strHint, mp_string& strInput, mp_int32 iInputLen)
{
    strInput.clear();
}

mp_void GetInputNotEmpty(const mp_string& strHint, mp_string& strInput, mp_int32 iInputLen)
{
    strInput = "Agent";
}

mp_int32 WriteFileFailed(mp_string& strFilePath, const vector<mp_string>& vecInput)
{
    return MP_FAILED;
}

mp_int32 WriteFileSucc(mp_string& strFilePath, const vector<mp_string>& vecInput)
{
    return MP_SUCCESS;
}

mp_int32 VerifyAgentUserFailed(mp_string& strUsrName)
{
    return MP_FAILED;
}

mp_int32 VerifyAgentUserSucc(mp_string& strUsrName)
{
    return MP_SUCCESS;
}

mp_int32 GetUidByUserNameFailed(const mp_string& strUserName, mp_int32& uid, mp_int32& gid)
{
    return MP_FAILED;
}

mp_int32 GetUidByUserNameSucc(const mp_string& strUserName, mp_int32& uid, mp_int32& gid)
{
    return MP_SUCCESS;
}

mp_int32 ChownFileFailed(const mp_string& strFileName, mp_int32 uid, mp_int32 gid)
{
    return MP_FAILED;
}

mp_int32 ChownFileSucc(const mp_string& strFileName, mp_int32 uid, mp_int32 gid)
{
    return MP_SUCCESS;
}

mp_int32 chmodFailed(const char *__file, __mode_t __mode)
{
    return -1;
}

mp_int32 chmodSucc(const char *__file, __mode_t __mode)
{
    return 0;
}

}

static mp_bool stubCheckAdminOldPwd(mp_void)
{
        return MP_TRUE;
}


static mp_int32 stubInputNginxInfo(mp_void)
{
    static int iCounter = 0;
    if (++iCounter <= 1)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}


static mp_int32 stubChgNginxInfo(mp_void)
{
    static int iCounter = 0;
    if (++iCounter <= 1)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

TEST_F(CChgNgxPwdTest, Handle)
{
    mp_int32 iRet = 0;
    ChgNgxPwd PwdObj;
    stub.set(&PBKDF2Hash, stub_return_success);
    stub.set(&CPassword::GetInput, stub_return_nothing);

    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CPassword::CheckAdminOldPwd, stubCheckAdminOldPwd);

    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    /* InputNginxInfo; */
    stub.set(&ChgNgxPwd::InputNginxInfo, stubInputNginxInfo);

    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    /* ChgNginxInfo; */
    stub.set(&ChgNgxPwd::ChgNginxInfo, stubChgNginxInfo);

    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_SUCCESS, iRet);

    return;
}

TEST_F(CChgNgxPwdTest, InputNginxInfo)
{
    mp_int32 iRet = 0;
    mp_string strCertificate;
    mp_string strKeyFile;
    mp_string strNewPwd;
    ChgNgxPwd PwdObj;

    stub.set(&CPassword::GetInput, stub_return_nothing);
    stub.set(&CMpFile::FileExist, stub_return_bool);

    PwdObj.InputNginxInfo(strCertificate, strKeyFile, strNewPwd);

    stub.set(&CPassword::ChgPwdNoCheck, stub_return_ret);
    PwdObj.InputNginxInfo(strCertificate, strKeyFile, strNewPwd);
    PwdObj.InputNginxInfo(strCertificate, strKeyFile, strNewPwd);

    return ;
}

static mp_int32 StubReadFile(mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    static int iCounter = 0;
    if (++iCounter <= 1)
    {
        return MP_FAILED;
    }
    else
    {
        vecOutput.push_back("asdfasdfdf");
        return MP_SUCCESS;
    }
}

TEST_F(CChgNgxPwdTest, ChgNginxInfoTest)
{
    ChgNgxPwd chgNgxPwd;
    mp_string strCertificate;
    mp_string strKeyFile;
    mp_string strNewPwd;
    
    stub.set(ADDR(CMpFile, FileExist), FileExistFalse);
    mp_int32 iRet = chgNgxPwd.ChgNginxInfo(strCertificate, strKeyFile, strNewPwd);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CMpFile, FileExist), FileExistTrue);
    stub.set(ADDR(CMpFile, ReadFile), ReadFileFailed);
    iRet = chgNgxPwd.ChgNginxInfo(strCertificate, strKeyFile, strNewPwd);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CMpFile, ReadFile), ReadFileSucc);
    iRet = chgNgxPwd.ChgNginxInfo(strCertificate, strKeyFile, strNewPwd);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(ChgHostSNTest, HandleTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);

    stub.set(ADDR(ChgHostSN, CheckUserPwd), CheckUserPwdFailed);
    mp_int32 iRet = ChgHostSN::Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ChgHostSN, CheckUserPwd), CheckUserPwdSucc);
    stub.set(ADDR(ChgHostSN, GetHostSNNum), GetHostSNNumFailed);
    iRet = ChgHostSN::Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ChgHostSN, GetHostSNNum), GetHostSNNumSucc);
    stub.set(ADDR(ChgHostSN, ModifyHostSN), ModifyHostSNFailed);
    iRet = ChgHostSN::Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ChgHostSN, ModifyHostSN), ModifyHostSNSucc);
    stub.set(ADDR(ChgHostSN, ChownHostSn), ChownHostSnFailed);
    iRet = ChgHostSN::Handle();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ChgHostSN, ChownHostSn), ChownHostSnSucc);
    iRet = ChgHostSN::Handle();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(ChgHostSNTest, GetHostSNNumTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    mp_string strInput;

    stub.set(ADDR(CPassword, GetInput), GetInputEmpty);
    mp_int32 iRet = ChgHostSN::GetHostSNNum(strInput);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CPassword, GetInput), GetInputNotEmpty);
    iRet = ChgHostSN::GetHostSNNum(strInput);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(ChgHostSNTest, ModifyHostSNTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    std::vector<mp_string> vecResult;
    mp_string strInput;

    stub.set(ADDR(CIPCFile, WriteFile), WriteFileFailed);
    mp_int32 iRet = ChgHostSN::ModifyHostSN(vecResult, strInput);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CIPCFile, WriteFile), WriteFileSucc);
    iRet = ChgHostSN::ModifyHostSN(vecResult, strInput);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(ChgHostSNTest, CheckUserPwdTest)
{
    Stub stub;

    stub.set(ADDR(CPassword, VerifyAgentUser), VerifyAgentUserFailed);
    mp_int32 iRet = ChgHostSN::CheckUserPwd();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CPassword, VerifyAgentUser), VerifyAgentUserSucc);
    iRet = ChgHostSN::CheckUserPwd();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(ChgHostSNTest, ChownHostSnTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    mp_string strInput = "Agent";

    stub.set(GetUidByUserName, GetUidByUserNameFailed);
    mp_int32 iRet = ChgHostSN::ChownHostSn(strInput);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(GetUidByUserName, GetUidByUserNameSucc);
    stub.set(ChownFile, ChownFileFailed);
    iRet = ChgHostSN::ChownHostSn(strInput);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ChownFile, ChownFileSucc);
    stub.set(chmod, chmodFailed);
    iRet = ChgHostSN::ChownHostSn(strInput);
    EXPECT_EQ(iRet, MP_FAILED);
    
    stub.set(ChmodFile, chmodSucc);
    stub.set(chmod, chmodSucc);
    iRet = ChgHostSN::ChownHostSn(strInput);
    EXPECT_EQ(iRet, MP_SUCCESS);
}
