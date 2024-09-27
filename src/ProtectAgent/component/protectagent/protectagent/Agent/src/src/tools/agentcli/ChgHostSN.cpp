#include "tools/agentcli/ChgHostSN.h"
#include "common/Defines.h"
#include "common/ConfigXmlParse.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "host/host.h"
#include "common/CSystemExec.h"
#include "securecom/Password.h"
using namespace std;
mp_string ChgHostSN::m_ChghostsnFile;

/* ------------------------------------------------------------
Description  : 修改HostSN
Return       : MP_SUCCESS -- 成功
Create By    : cwx348302 chenxiaolei 2016-06-22
------------------------------------------------------------- */
mp_int32 ChgHostSN::Handle()
{
    COMMLOG(OS_LOG_DEBUG, "Begin to Chg HostSN.");

    // 用户校验
    mp_int32 iRet = ChgHostSN::CheckUserPwd();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_DEBUG, "Failed in func ChgHostSN::CheckUserPwd.");
        return iRet;
    }

    // 获取新输入的HostSN号
    mp_string strInput;
    iRet = ChgHostSN::GetHostSNNum(strInput);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_DEBUG, "Failed in func ChgHostSN::GetHostSNNum.");
        return iRet;
    }

    // 修改该HostSN的值
    vector<mp_string> vecResult;
    iRet = ChgHostSN::ModifyHostSN(vecResult, strInput);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_DEBUG, "Failed in func ChgHostSN::ModifyHostSN.");
        return iRet;
    }

    // 修改HostSN文件的权限和所属
    iRet = ChgHostSN::ChownHostSn(m_ChghostsnFile);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Change HostSN file auth failed.");
        return MP_FAILED;
    }

    printf("HostSN changed succ!\n");
    COMMLOG(OS_LOG_DEBUG, "Write HostSN file succ.");

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 从终端获取HostSN号
Output       : 获取到的HostSN号
Return       : MP_SUCCESS -- 成功
Create By    : cwx348302 chenxiaolei 2016-06-22
------------------------------------------------------------- */
mp_int32 ChgHostSN::GetHostSNNum(mp_string& strInput)
{
    mp_uint32 InputTimes = 0;

    // 运行3次输入用户密码
    while (InputTimes <= MAX_FAILED_COUNT) {
        printf("%s", INPUT_HOSTSN_CHG);
        CPassword::GetInput(INPUT_HOSTSN_CHG, strInput, HOSTSN_LEN);
        if (strInput == "") {
            printf("The input HostSN is empty, Retry! .\n");
            COMMLOG(OS_LOG_ERROR, "Input HostSN is incorrectly.");
            InputTimes++;
            continue;
        } else {
            break;
        }
    }

    if (InputTimes > MAX_FAILED_COUNT) {
        printf("%s.\n", "Input incorrect HostSN more than 3 times, exit!");
        COMMLOG(OS_LOG_ERROR, "Input incorrect HostSN more than 3 times.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 修改HostSN号
Return       : MP_SUCCESS -- 成功
Create By    : cwx348302 chenxiaolei 2016-06-22
------------------------------------------------------------- */
mp_int32 ChgHostSN::ModifyHostSN(vector<mp_string>& vecResult, mp_string& strInput)
{
    vecResult.clear();
    vecResult.push_back(strInput);
    m_ChghostsnFile = CPath::GetInstance().GetConfFilePath(HOSTSN_FILE);
    mp_int32 iRet = CIPCFile::WriteFile(m_ChghostsnFile, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Write HostSN file failed, iRet = %d, size of vecResult is %d.", iRet, vecResult.size());
        return iRet;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 用户校验
Return       : MP_SUCCESS -- 成功
Create By    : cwx348302 chenxiaolei 2016-06-22
------------------------------------------------------------- */
mp_int32 ChgHostSN::CheckUserPwd()
{
    mp_string strUsrName;
    return CPassword::VerifyAgentUser(strUsrName);
}

/* ------------------------------------------------------------
Function Name: ChownResult
Description  : 修改HOSTSN文件的所属组权限和执行权限，改成rdadmin用户，-rw-------执行权限
Others       :-------------------------------------------------------- */
mp_int32 ChgHostSN::ChownHostSn(mp_string& strInput)
{
#ifndef WIN32
    mp_int32 uid(-1), gid(-1);
    mp_string strBaseFileName = BaseFileName(strInput);

    // 获取rdadmin用户的uid和gid
    mp_int32 iRet = GetUidByUserName(AGENT_RUNNING_USER, uid, gid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get user(%s) uid and gid failed.", AGENT_RUNNING_USER.c_str());
        CMpFile::DelFile(strInput);
        return MP_FAILED;
    }

    // 设置rdadmin的uid和gid
    iRet = ChownFile(strInput, uid, gid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Chown file failed, file %s.", strBaseFileName.c_str());
        CMpFile::DelFile(strInput);
        return MP_FAILED;
    }

    // 修改用户执行权限
    if (ChmodFile(strInput, S_IRUSR | S_IWUSR) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "chmod file failed, file %s.", strInput.c_str());
        CMpFile::DelFile(strInput);
        return MP_FAILED;
    }

    return MP_SUCCESS;
#else
    mp_string strCommand = "cmd.exe /c echo Y | cacls.exe \"" + strInput + "\" /E /R Users > c:\\nul";

    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCommand);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "cacls hostsn file failed %d.", iRet);
        CMpFile::DelFile(strInput);
        return iRet;
    }

    return MP_SUCCESS;
#endif
}
