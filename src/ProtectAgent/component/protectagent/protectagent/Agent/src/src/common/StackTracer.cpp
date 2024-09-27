#include "common/StackTracer.h"
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <malloc.h>
#include <ctime>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sys/types.h>
#if defined(LINUX)
#include <execinfo.h>
#elif defined(AIX)

#elif defined(HP_UX_IA)
#include <unwind.h>
#endif
#include "securec.h"
#include "common/Path.h"

typedef void (*SignalHandlerType)(int, siginfo_t *, void *);

namespace hcp {
#ifdef WIN32
void InstallSignalHandler(int signum, void *handler)
#else
void InstallSignalHandler(int signum, SignalHandlerType handler)
#endif
{
#if defined(LINUX) || defined(SOLARIS)
    struct sigaction sa;
    memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
    (void)sigaction(signum, &sa, NULL);
#endif
}

void UninstallSignalHandler(int signum)
{
#if defined(LINUX) || defined(SOLARIS)
    struct sigaction sa;
    (void)sigaction(signum, NULL, &sa);
    sa.sa_handler = SIG_DFL;
    (void)sigaction(signum, &sa, NULL);
#endif
}
}  // namespace hcp

namespace {
const mp_int32 NUM_64 = 64;
const mp_int32 NUM_32 = 32;
const mp_int32 NUM_1000 = 1000;
}  // namespace

void *StackTracer::btarray[NUM_32];
StackTracer::StackTracer()
{
    InstallSignalHandler();
    // note: stand I/O and error I/O has been redirected to filename by script, so we use printf() will print to
    // filename instead of screen.

    std::time_t timenow = std::time(NULL);
    char strtime[NUM_64] = {0};

    std::tm *localnow = std::localtime(&timenow);
    if (localnow != NULL) {
        std::strftime(strtime, sizeof(strtime), "[%d-%b-%Y %H:%M:%S]", localnow);
    }

    std::cout << "[start program] at:" << strtime << std::endl;
}

StackTracer::~StackTracer()
{
    UninstallSignalHandler();
}

void StackTracer::InstallSignalHandler()
{
    hcp::InstallSignalHandler(SIGSEGV, (SignalHandlerType)StackTracer::SignalHandler);
    hcp::InstallSignalHandler(SIGFPE, (SignalHandlerType)StackTracer::SignalHandler);
    hcp::InstallSignalHandler(SIGABRT, (SignalHandlerType)StackTracer::SignalHandler);
    hcp::InstallSignalHandler(SIGBUS, (SignalHandlerType)StackTracer::SignalHandler);
    hcp::InstallSignalHandler(SIGTERM, (SignalHandlerType)StackTracer::SignalHandler);
}

void StackTracer::UninstallSignalHandler()
{
    hcp::UninstallSignalHandler(SIGSEGV);
    hcp::UninstallSignalHandler(SIGFPE);
    hcp::UninstallSignalHandler(SIGABRT);
    hcp::UninstallSignalHandler(SIGBUS);
    hcp::UninstallSignalHandler(SIGTERM);
}

void StackTracer::OutputMaps(std::vector<mp_string>& stackStream)
{
#if defined(LINUX)
    std::ostringstream oss;
    oss << "cat /proc/" << getpid() << "/maps";
    mp_string strCmd = oss.str();

    // 直接封装的pid，没有命令注入的风险
    FILE* pStream = popen(strCmd.c_str(), "r");
    if (pStream == NULL) {
        oss.str("");
        oss <<"export stack map failed, errorcode=" << errno;
        stackStream.push_back(oss.str());
        return;
    }

    oss.str("");
    oss <<"==================process " << getpid() <<" maps==================";
    stackStream.push_back(oss.str());

    while (!feof(pStream)) {
        mp_char tmpBuf[NUM_1000] = {0};
        mp_char* cRet = fgets(tmpBuf, sizeof(tmpBuf), pStream);
        if (cRet == NULL) {}
        if (strlen(tmpBuf) > 0) {
            tmpBuf[strlen(tmpBuf) - 1] = 0;  // 去掉获取出来的字符串末尾的'\n'
        }

        mp_bool bFlag = (tmpBuf[0] == 0) || (tmpBuf[0] == '\n');
        if (bFlag) {
            continue;
        }

        mp_string tmpStr = tmpBuf;
        stackStream.emplace_back(tmpStr);
    }

    if (pclose(pStream) == -1) {
        oss.str("");
        oss <<"close /proc/pid/maps failed, error=" <<errno;
        stackStream.push_back(oss.str());
    }
#endif
}

int StackTracer::SignalHandler(int signum, void *siginfo, void *ucontext)
{
    static const int width = 2;
    (void)siginfo;   // unused parameters
    (void)ucontext;  // unused parameters
    if ((signum != SIGSEGV) && (signum != SIGFPE) && (signum != SIGABRT) && (signum != SIGBUS) && (signum != SIGTERM)) {
        return 0;
    }
    
    mp_string stackFile = CPath::GetInstance().GetLogPath() + PATH_SEPARATOR + AGENT_LOG_NAME;
    // write stack to timestamp file
    std::vector<mp_string> stackStream;
    OutputMaps(stackStream);

#if defined(LINUX)
    // solaris reference https://docs.oracle.com/cd/E26502_01/html/E29034/backtrace-3c.html
    size_t size = backtrace(btarray, NUM_32);
    mp_char **stackInfo = backtrace_symbols(btarray, size);
    if (stackInfo == NULL) {
        stackStream.push_back("backtrace_symbols failed.");
        WriteStackContent(stackFile, stackStream);
        hcp::UninstallSignalHandler(SIGABRT);
        std::abort();
        return 0;
    }

    stackStream.push_back("==================stack list==================");
    for (mp_int32 j = 0; j < size; j++) {
        std::ostringstream oss;
        oss << "  [" << std::setfill('0') << std::setw(width) << j << "]" << stackInfo[j];
        stackStream.push_back(oss.str());
    }
    free(stackInfo);
    
#elif defined(AIX)
    // reference https://www.ibm.com/support/knowledgecenter/ssw_aix_72/m_bostechref/mt_trce.html
    // commit data like mt__trce(STDOUT_FILENO, signal, ucontext, 0);
#elif defined(HP_UX_IA)
    // reference https://docstore.mik.ua/manuals/hp-ux/en/B2355-60130/U_STACK_TRACE.3X.html
    U_STACK_TRACE();
#else
    std::ostringstream oss;
    oss <<"Unimplement function:" << signum;
    stackStream.push_back(oss.str());
#endif

    WriteStackContent(stackFile, stackStream);
    hcp::UninstallSignalHandler(SIGABRT);
    std::abort();
    return 0;
}

/* ------------------------------------------------------------
Function Name: WriteStackContent
Description  : 将string的vector中的元素逐条取出并切到文件中，已经到了core流程中，如果写入失败也无法输出错误，只能返回
Others       :-------------------------------------------------------- */
void StackTracer::WriteStackContent(const mp_string& stackFile, const std::vector<mp_string>& vecInput)
{
    // 已经到了core流程中，如果写入失败也无法输出错误，只能返回
    FILE* pFile = fopen(stackFile.c_str(), "a+");
    if (pFile == NULL) {
        return;
    }

    for (std::vector<mp_string>::const_iterator iter = vecInput.begin(); iter != vecInput.end(); ++iter) {
        fprintf(pFile, "%s\n", iter->c_str());
    }

    // 已经到了core流程中，如果写入失败也无法输出错误，只能返回
    (void) fflush(pFile);
    (void) fclose(pFile);
}
