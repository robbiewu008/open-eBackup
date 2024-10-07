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
#include "system/System.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <securec.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/xpressive/xpressive_dynamic.hpp>
#include "common/EnvVarManager.h"
#ifndef WIN32
#include <sys/sysinfo.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#ifdef __linux__
#include <ifaddrs.h>
#endif
#else
#include <locale>
#include <codecvt>
#include <windows.h>
#endif

#include <string>
#include "config_reader/ConfigIniReader.h"

using boost::property_tree::ptree;
using namespace std;

#ifndef WIN32

namespace Module {
    #define DEFAULT_CUR_PATH "/DataBackup/ProtectClient/ProtectClient-E/"
    string getSafeCmd(const severity_level &severity, const string &moduleName, const size_t &requestID,
        const string &cmd)
    {
        string outputCmd = "";
        try {
            SensitiveInfoWiper wiper(cmd);
            wiper("password", "WIPED");
            wiper("passwd", "WIPED");
            wiper("pwd", "WIPED");
            wiper("PWD", "WIPED");
            wiper("PASSWORD", "WIPED");
            wiper("PASSWD", "WIPED");
            outputCmd = wiper.toString();
        } catch (boost::exception &bex) {
            HCP_Logger(severity, moduleName, requestID) << "Exception happened." << HCPENDLOG;
        }

        return outputCmd;
    }

    int RunLoggedSystemCommand(const severity_level& severity, const string& moduleName, const size_t& requestID,
        const string& cmd, vector<string> params, bool bSec)
    {
        bSec = bSec;
        if (!checkParam(params)) {
            HCP_Logger(severity, moduleName, requestID) << "######The params of cmd str  "
                                                        << " include  illegal chars(| ; & $ > <)." << HCPENDLOG;
            return 1;
        }
        string combineStr;
        if (!combineCmdstr(severity, moduleName, requestID, cmd, params, combineStr)) {
            HCP_Logger(severity, moduleName, requestID)
                << "######Re combine str failed. The input str is: " << WIPE_SENSITIVE(cmd) << HCPENDLOG;
            return 1;
        }

        // HCP_Logger(severity, moduleName, requestID) << "######The combine str is:" << combineStr<< HCPENDLOG;

        int retv = 0;
        char* cwd = getcwd(nullptr, 0);
        if (0 != chdir("/")) {
            HCP_Logger(ERR, moduleName, requestID) << "chdir / failed" << HCPENDLOG;
            if (cwd != nullptr) {
                (void)chdir(cwd);
                free(cwd);
                cwd = nullptr;
            }
            return 1;
        }

        string cmdStr = combineStr;
        if (0 == combineStr.find("sudo ") || 0 == combineStr.find("/usr/bin/sudo")) {
            cmdStr = "export LD_LIBRARY_PATH=;" + combineStr;
        }

        SensitiveInfoWiper wiper(cmd);
        wiper("password", "WIPED");
        wiper("passwd", "WIPED");
        wiper("pwd", "WIPED");
        wiper("PWD", "WIPED");
        wiper("PASSWORD", "WIPED");
        wiper("PASSWD", "WIPED");

        unsigned int runShellType = ConfigReader::getUint("MicroService", "RunShellByBoost");
        // Modify for security, just print the frame of command,not print the param.
        try {
            bp::ipstream is_stdout;
            bp::ipstream is_stderr;
            bp::child c = StartChild(cmdStr, is_stdout, is_stderr, runShellType);
            GET_LINES
        } catch (exception& e) {
            HCP_Logger(CRIT, moduleName, requestID) << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
            retv = 1;
        } catch (...) {
            HCP_Logger(CRIT, moduleName, requestID) << "Unknown exception" << HCPENDLOG;
            retv = 1;
        }

        if (cwd != nullptr) {
            (void)chdir(cwd);
            free(cwd);
            cwd = nullptr;
        }
        
        return retv;
    }

    bool combineCmdstr(const severity_level& severity, const string& moduleName, const size_t& requestID,
        const string& inputcmd, vector<string> params, string& combineCmd)
    {
        unsigned int count = 0;
        for (unsigned int i = 0; i < inputcmd.length(); i++) {
            if (inputcmd[i] == '?') {
                count++;
            }
        }

        // The param is not match
        if (count != params.size()) {
            HCP_Logger(severity, moduleName, requestID)
                << "######The param is not match, the cmd need " << count
                << " params. But the parma list size is:" << params.size() << HCPENDLOG;
            return false;
        }

        combineCmd = inputcmd;
        string::size_type pos = combineCmd.size() - 1;
        for (unsigned int i = count; i > 0; i--) {
            pos = combineCmd.find_last_of("?", pos);
            if (pos != string::npos) {
                combineCmd = combineCmd.replace(pos, 1, params[i - 1]);
            }
            // Forward one step and continue
            pos = pos - 1;
        }

        return true;
    }

    int runShellCmdWithOutput(const severity_level& severity, const string& moduleName,
        const size_t& requestID, const string& cmd, const vector<string> params,
        vector<string>& cmdoutput, vector<string>& stderroutput)
    {
        string safecmd = getSafeCmd(severity, moduleName, requestID, cmd);
        stringstream outstring;

        unsigned int runShellType = ConfigReader::getUint("MicroService", "RunShellByBoost");
        HCP_Logger(severity, moduleName, requestID) << "stdout of @@@ " << WIPE_SENSITIVE(safecmd)
            << " @@@" << HCPENDLOG;
        int ret = BaseRunShellCmdWithOutput(
            moduleName, requestID, cmd, params, cmdoutput, stderroutput, outstring, runShellType);
        if (ret != 0) {
            HCP_Logger(ERR, moduleName, requestID) << "run shell fail, error:"
                << WIPE_SENSITIVE(outstring.str()) << HCPENDLOG;
        }
        return ret;
    }

    ostream& operator<<(ostream& strm, const SensitiveInfoWiper& obj)
    {
        strm<<obj.toString();
        return strm;
    }

    std::string FormatFullUrl(const std::string& fullUrl)
    {
        std::string baseUrl = "";
        std::string headUrl = "";

        int baseUrloffset = 3;
        std::string::size_type head = fullUrl.find("://");
        if (head != std::string::npos) {
            baseUrl = fullUrl.substr(head + baseUrloffset);
            headUrl = fullUrl.substr(0, head + baseUrloffset);
        } else {
            return fullUrl;
        }

        std::string::size_type checkend = fullUrl.find('/', head + baseUrloffset);
        if (checkend == std::string::npos) {
            return fullUrl;
        }
        std::string iPAndPortString = fullUrl.substr(head + baseUrloffset, checkend - head - baseUrloffset);

        int colonsCnt = 2;
        unsigned int count = 0;
        for (unsigned int i = 0; i < iPAndPortString.length(); i++) {
            if (iPAndPortString[i] == ':') {
                count++;
                if (count >= colonsCnt) {
                    break;
                }
            }
        }
        if (count <= 1) {
            // this is IPV4
            return fullUrl;
        } else {
            // this is IPV6
            if (iPAndPortString.find('[') != std::string::npos) {
                return fullUrl;
            }
            std::string::size_type last_delim = iPAndPortString.find_last_of(':');
            std::string ipstr = baseUrl.substr(0, last_delim);
            std::string portstr = baseUrl.substr(last_delim + 1);
            std::string ipv6str = headUrl + "[" + ipstr + "]" + ":" + portstr;
            HCP_Log(DEBUG, "System") << "new URL=" << ipv6str << HCPENDLOG;
            return ipv6str;
        }
    }

    // kill process and syc log
    void killAfterSyncLog()
    {
        int ret = 0;
        HCP_Logger(ERR, "killAfterSyncLog", 0) << "The process(" << getpid() << "), will be killed." << HCPENDLOG;
        // No need to Sync log content to logfile, because CLogger will open->write->close log file every time
        ret = kill(getpid(), SIGKILL);
        if (0 != ret) {
            HCP_Logger(ERR, "killAfterSyncLog", 0) << "Kill process with result(" << ret << ") finished." << HCPENDLOG;
        }
    }
    // print output and error info
    void printCmdOutputInfo(const std::string& moduleName, const std::size_t& requestID,
                            std::vector<std::string>& cmdoutput, std::vector<std::string>& stderroutput)
    {

        for (std::vector<std::string>::size_type i = 0; i < cmdoutput.size(); ++i) {
            HCP_Logger(INFO, moduleName, requestID) << "output: " << WIPE_SENSITIVE(cmdoutput[i]) << HCPENDLOG;
        }

        for (std::vector<std::string>::size_type i = 0; i < stderroutput.size(); ++i) {
            HCP_Logger(ERR, moduleName, requestID) << "stderr: " << WIPE_SENSITIVE(stderroutput[i]) << HCPENDLOG;
        }
    }
    void getCurrentFilePath(boost::filesystem::path& fullFileName, const std::string& moduleName, const uint64_t& requestID)
    {
#define PATH_BUFF_SIZE 512
        // the path is relative to executable location, not to current working directory
        char szPath[PATH_BUFF_SIZE] = {'\0'};
#ifdef __WINDOWS__
        GetModuleFileName(NULL, szPath, sizeof(szPath));
#else
        int rv = (int)readlink("/proc/self/exe", szPath, PATH_BUFF_SIZE - 1);
        // for compiling in release mode:
        if (rv <= 0) {
            szPath[PATH_BUFF_SIZE - 1] = '\0';
            std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
            std::string defaultCurPath = agentHomedir + DEFAULT_CUR_PATH;
            HCP_Logger(CRIT, moduleName, requestID) << "readlink return:" << rv << ", return the default path" << HCPENDLOG;
            (void)strncpy_s(szPath, PATH_BUFF_SIZE, defaultCurPath.c_str(), strlen(defaultCurPath.c_str()));
        } else {
            szPath[rv] = '\0';
        }
#endif

        try {
            if (rv <= 0) {
                fullFileName = boost::filesystem::system_complete(szPath);
            } else {
                fullFileName = boost::filesystem::system_complete(szPath).parent_path();
            }
        } catch (boost::filesystem::filesystem_error& e) {
            HCP_Logger(CRIT, moduleName, requestID) << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
        }
    }

    void getCurrentFilePath(std::string& fullFileName, const std::string& moduleName, const uint64_t& requestID)  //lint !e121
    {
        std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
        try {
            boost::filesystem::path ffullFileName;
            getCurrentFilePath(ffullFileName, moduleName, requestID);
            fullFileName = ffullFileName.string();
            std::size_t pos = fullFileName.find_last_of('/');
            if (string::npos == pos)
                fullFileName = agentHomedir + DEFAULT_CUR_PATH;
        } catch (boost::filesystem::filesystem_error& e) {
            HCP_Logger(CRIT, moduleName, requestID) << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
            HCP_Logger(CRIT, moduleName, requestID) << "Set current path to default" << HCPENDLOG;
            fullFileName = agentHomedir + DEFAULT_CUR_PATH;
        }
    }
}
#else
namespace Module{
    AGENT_API int ExecWinCmd(const string& cmd, uint32_t& retCode)
    {
        list<string> lstContents;
        int iRet = 0;
        STARTUPINFO stStartupInfo;
        PROCESS_INFORMATION stProcessInfo;
        DWORD dwCode = 0;

        ZeroMemory(&stStartupInfo, sizeof(stStartupInfo));
        stStartupInfo.cb = sizeof(STARTUPINFO);
        stStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
        stStartupInfo.wShowWindow = SW_HIDE;
        ZeroMemory(&stProcessInfo, sizeof(stProcessInfo));

        using ConvertTypeX = std::codecvt_utf8_utf16<wchar_t>;
        std::wstring_convert<ConvertTypeX, wchar_t> converterX;
        wstring wCmd = converterX.from_bytes(cmd);
        if (!CreateProcess(NULL, (LPWSTR)wCmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &stStartupInfo, &stProcessInfo)) {
            retCode = GetLastError();
            printf("CreateProcess failed: %d\n", retCode);
            return -1;
        }

        if (WAIT_TIMEOUT == WaitForSingleObject(stProcessInfo.hProcess, 1000 * 3600)) {
            retCode = 1;
        }
        else {
            GetExitCodeProcess(stProcessInfo.hProcess, &dwCode);
            retCode = dwCode;
        }

        CloseHandle(stProcessInfo.hProcess);
        CloseHandle(stProcessInfo.hThread);

        return 0;
    }

    int runShellCmdWithOutput(const severity_level& severity, const string& moduleName,
        const size_t& requestID, const string& cmd, const vector<string> params,
        vector<string>& cmdoutput, vector<string>& stderroutput)
    {
        ERRLOG("WIN32 NOT IMPLMENTED");
        return 1;
    }
}
#endif
