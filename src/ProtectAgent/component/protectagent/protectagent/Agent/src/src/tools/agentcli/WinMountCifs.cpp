#include "tools/agentcli/WinMountCifs.h"
#include "common/Log.h"
#include "securecom/CryptAlg.h"

#ifdef WIN32
namespace {
    constexpr mp_int32 CMD_WAITING_TIME = 100;
    constexpr mp_int32 CMD_TIMEOUT = 30000;
}

mp_int32 WinMountCifs::Handle(const mp_string& strCMD)
{
    INFOLOG("strCMD=%s.", strCMD.c_str());
    if (CreateProcess(NULL, TEXT((LPTSTR)strCMD.c_str()), NULL, NULL, false, 0, NULL, NULL,
        &stStartupInfo, &stProcessInfo) == MP_FALSE) {
        iErr = GetOSError();
        ERRLOG("CreateProcess failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    DoSleep(CMD_WAITING_TIME);
    if (FreeConsole() == MP_FALSE) {
        iErr = GetOSError();
        ERRLOG("FreeConsole failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        TerminateProcess(stProcessInfo.hProcess, 1);
        return MP_FAILED;
    }
    DoSleep(CMD_WAITING_TIME);
    if (AttachConsole(stProcessInfo.dwProcessId) == MP_FALSE) {
        iErr = GetOSError();
        ERRLOG("AttachConsole failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        TerminateProcess(stProcessInfo.hProcess, 1);
        return MP_FAILED;
    }
    DoSleep(CMD_WAITING_TIME);
    if (WriteInput() != MP_SUCCESS) {
        TerminateProcess(stProcessInfo.hProcess, 1);
        ERRLOG("Write input Fail.");
        return MP_FAILED;
    }
    DWORD dwCode = MP_FAILED;
    if (WAIT_TIMEOUT == WaitForSingleObject(stProcessInfo.hProcess, CMD_TIMEOUT)) {
        ERRLOG("WaitForSingleObject timeout");
    } else {
        GetExitCodeProcess(stProcessInfo.hProcess, &dwCode);
        INFOLOG("GetExitCodeProcess is %d", dwCode);
    }
    TerminateProcess(stProcessInfo.hProcess, 1);
    return dwCode;
}

mp_int32 WinMountCifs::GetPwd(mp_string &strPwd)
{
    auto pEnv = getenv("AGENT_ENV_AuthPassword");
    if (pEnv == nullptr) {
        ERRLOG("Getenv AGENT_ENV_AuthPassword failed.");
        return MP_FAILED;
    }
    _putenv_s("AGENT_ENV_AuthPassword", "");
    DecryptStr(pEnv, strPwd);
    if (strPwd.empty()) {
        ERRLOG("Decrypt env failed.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 WinMountCifs::WriteInput()
{
    HANDLE fStdIn = GetStdHandle(STD_INPUT_HANDLE);
    if (fStdIn == INVALID_HANDLE_VALUE) {
        iErr = GetOSError();
        ERRLOG("GetStdHandle failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    mp_string strPwd;
    if (GetPwd(strPwd) != MP_SUCCESS) {
        CloseHandle(fStdIn);
        return MP_FAILED;
    }
    strPwd += "\r\n";
    INPUT_RECORD *buf = new INPUT_RECORD[strPwd.size() + 2];
    for (size_t i = 0; i < strPwd.size(); i++) {
        buf[i].EventType = KEY_EVENT;
        buf[i].Event.KeyEvent.bKeyDown = TRUE;
        buf[i].Event.KeyEvent.wRepeatCount = 1;
        buf[i].Event.KeyEvent.wVirtualKeyCode = 0;
        buf[i].Event.KeyEvent.wVirtualScanCode = 0;
        buf[i].Event.KeyEvent.dwControlKeyState = 0;
        if (strPwd[i] == '\n') {
            buf[i].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
            buf[i].Event.KeyEvent.wVirtualScanCode = MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC);
            buf[i].Event.KeyEvent.uChar.UnicodeChar = '\n';
        } else {
            buf[i].Event.KeyEvent.uChar.UnicodeChar = strPwd[i];
        }
    }
    DWORD wNum;
    if (WriteConsoleInput(fStdIn, buf, strPwd.size(), &wNum) == MP_FALSE) {
        delete[] buf;
        ClearString(strPwd);
        CloseHandle(fStdIn);
        iErr = GetOSError();
        ERRLOG("WriteConsoleInput failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    delete[] buf;
    ClearString(strPwd);
    CloseHandle(fStdIn);
    return MP_SUCCESS;
}
#endif // WIN32