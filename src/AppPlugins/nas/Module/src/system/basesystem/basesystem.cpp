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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <securec.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string_regex.hpp>
#ifndef WIN32
#include <unistd.h>
#endif
#include <sys/types.h>
#include "system/basesystem/basesystem.hpp"

// 禁止在这个文件添加的函数包含日志打印，参数获取;
// 同样也是禁止在这里去调用包含日志打印，参数获取的函数;
// 因为这样会引起在打印日志时引起死锁;

using boost::property_tree::ptree;

namespace Module {
    const int VFORK_FAILED = 1;
#ifndef WIN32
    std::mutex cmd_mutex;
    bp::child StartChild(const std::string &cmd, bp::ipstream &is_out, bp::ipstream &is_err,
        const unsigned int &runShellType)
    {
        std::string exec = "/bin/bash";

        std::vector<std::string> args;
        args.push_back("-c");
        args.push_back(cmd);

        auto env = boost::this_process::environment();
        unsigned int boostPosixVfork = 2;
        if (runShellType == boostPosixVfork) {
            return bp::child(exec, args, bp::std_out > is_out, bp::std_err > is_err, bp::posix::use_vfork, env);
        }
        return bp::child(exec, args, bp::std_out > is_out, bp::std_err > is_err, env);
    }

    int StartBoostChild(std::string &cmdStr, std::vector<std::string> &cmdoutput,
        std::vector<std::string> &stderroutput, const unsigned int &runShellType)
    {
        boost::process::ipstream is_stdout;
        boost::process::ipstream is_stderr;

        boost::process::child c = StartChild(cmdStr, is_stdout, is_stderr, runShellType);

        std::string line;

        while (std::getline(is_stdout, line)) {
            cmdoutput.push_back(line);
        }

        while (std::getline(is_stderr, line)) {
            stderroutput.push_back(line);
        }

        c.wait();
        return c.exit_code();
    }

    void ReadPipe(std::vector<std::string> &output, int pipefd)
    {
        char buf;
        while (::read(pipefd, &buf, 1) > 0) {
            if (buf == '\n') {
                continue;
            }
            std::string tmp;
            tmp.append(&buf, 1);
            while (::read(pipefd, &buf, 1) > 0) {
                if (buf == '\n') {
                    break;
                } else {
                    tmp.append(&buf, 1);
                }
            }
            output.push_back(tmp);
        }
    }

    int ForkFail(const int stdOutputfd[], int outputNum, const int stdERRfd[], int errNum,
        std::stringstream &outstring)
    {
        int vforkERRNO = errno;
        outstring << "vfork fail"
                  << " errno:" << vforkERRNO << std::endl;
        close(stdOutputfd[0]);
        close(stdOutputfd[1]);
        close(stdERRfd[0]);
        close(stdERRfd[1]);
        return VFORK_FAILED;
    }

    int ForkSuccess(const int stdOutputfd[], const int stdERRfd[], std::stringstream &outstring,
        std::vector<std::string> &stdOUTPUTs, std::vector<std::string> &stdERRs, pid_t childPID)
    {
        int status = 0;
        close(stdOutputfd[1]);
        close(stdERRfd[1]);
        ReadPipe(stdOUTPUTs, stdOutputfd[0]);
        ReadPipe(stdERRs, stdERRfd[0]);
        close(stdOutputfd[0]);
        close(stdERRfd[0]);
        while (waitpid(childPID, &status, 0) < 0) {
            if (errno != EINTR) {
                return VFORK_FAILED;
            }
        }
        return status;
    }

    int StartvForkChild(std::vector<std::string> &stdOUTPUTs, std::vector<std::string> &stdERRs, const std::string &cmd,
        std::stringstream &outstring)
    {
        int iRet = EOK;
        if (cmd.size() == 0) {
            outstring << "cmd is empty" << std::endl;
            return VFORK_FAILED;
        }
        int stdOutputfd[2], stdERRfd[2];
        if (pipe(stdOutputfd) != 0) {
            outstring << "pipe stdoutput failed errno:" << errno << std::endl;
            return VFORK_FAILED;
        }
        if (pipe(stdERRfd) != 0) {
            ::close(stdOutputfd[0]);
            ::close(stdOutputfd[1]);
            outstring << "pipe stderr failed errno:" << errno << std::endl;
            return VFORK_FAILED;
        }
        pid_t childPID = vfork();
        if (childPID == 0) {
            close(stdOutputfd[0]);
            close(stdERRfd[0]);
            if (stdOutputfd[1] != STDOUT_FILENO) {
                dup2(stdOutputfd[1], STDOUT_FILENO);
                close(stdOutputfd[1]);
            }
            if (stdERRfd[1] != STDERR_FILENO) {
                dup2(stdERRfd[1], STDERR_FILENO);
                close(stdERRfd[1]);
            }
            execl("/bin/bash", "bash", "-c", cmd.c_str(), (char *) 0);
            _exit(127);
        } else if (childPID < 0) {
            return ForkFail(stdOutputfd, sizeof(stdOutputfd), stdERRfd, sizeof(stdERRfd), outstring);
        } else {
            return ForkSuccess(stdOutputfd, stdERRfd, outstring, stdOUTPUTs, stdERRs, childPID);
        }
    }

    bool checkParam(std::vector<std::string> params)
    {
        std::string special_str = "|;&$><`\\!\n";
        for (unsigned int i = 0; i < params.size(); i++) {
            if (params[i].find_first_of(special_str) != std::string::npos) {
                return false;
            }
        }
        return true;
    }

    static bool combineCmdstr(const std::string &moduleName, const std::size_t &requestID, const std::string &inputcmd,
        std::vector<std::string> params, std::string &combineCmd)
    {
        unsigned int count = 0;
        for (unsigned int i = 0; i < inputcmd.length(); i++) {
            if (inputcmd[i] == '?') {
                count++;
            }
        }

        if (count != params.size()) {
            return false;
        }

        combineCmd = inputcmd;
        std::string::size_type pos = combineCmd.size() - 1;
        for (unsigned int i = count; i > 0; i--) {
            pos = combineCmd.find_last_of("?", pos);
            if (pos != std::string::npos) {
                combineCmd = combineCmd.replace(pos, 1, params[i - 1]);
            }
            // Forward one step and continue
            pos = pos - 1;
        }

        return true;
    }

    int BaseRunShellCmdWithOutputWithOutLock(const std::string &moduleName, const std::size_t &requestID,
        const std::string &cmd, const std::vector<std::string> params, std::vector<std::string> &cmdoutput,
        std::vector<std::string> &stderroutput, std::stringstream &outstring, const unsigned int &runShellType)
    {
        if (!checkParam(params)) {
            outstring << "check param error";
            return 1;
        }

        std::string combineStr;
        if (!combineCmdstr(moduleName, requestID, cmd, params, combineStr)) {
            outstring << "combineCmdstr error";
            return 1;
        }

        int retv = 0;
        char* cwd = getcwd(nullptr, 0);
        if (chdir("/") != 0) {
            outstring << "chdir error";
            if (cwd != nullptr) {
                (void)chdir(cwd);
                free(cwd);
                cwd = nullptr;
            }
            return 1;
        }

        std::string cmdStr = combineStr;
        if (combineStr.find("sudo ") == 0
            || combineStr.find("/usr/bin/sudo") == 0) {
            cmdStr = "export LD_LIBRARY_PATH=;" + combineStr;
        }

        try {
            if (runShellType != 0) {
                retv = StartBoostChild(cmdStr, cmdoutput, stderroutput, runShellType);
            } else {
                retv = StartvForkChild(cmdoutput, stderroutput, cmdStr, outstring);
            }
        }
        catch (std::exception &e) {
            outstring << "catch exception:" << e.what();
            retv = 1;
        }
        catch (...) {
            outstring << "catch exception";
            retv = 1;
        }

        if (cwd != nullptr) {
            (void)chdir(cwd);
            free(cwd);
            cwd = nullptr;
        }

        return retv;
    }

    // 此函数不能打印日志，只能给日志模块调用。为了防止转储压缩时死锁;
    int BaseRunShellCmdWithOutput(const std::string &moduleName, const std::size_t &requestID,
        const std::string &cmd, const std::vector<std::string> params, std::vector<std::string> &cmdoutput,
        std::vector<std::string> &stderroutput, std::stringstream &outstring, const unsigned int &runShellType)
    {
        std::lock_guard<std::mutex> guard(cmd_mutex);
        return BaseRunShellCmdWithOutputWithOutLock(moduleName, requestID, cmd, params, cmdoutput, stderroutput,
            outstring, runShellType);
    }
#endif
}

