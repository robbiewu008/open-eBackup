#ifndef BOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC
#define BOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC
#endif
#ifndef MODULE_SYSTEM_HPP
#define MODULE_SYSTEM_HPP

#include "define/Defines.h"
#include "define/Types.h"
#include "boost/filesystem.hpp"
#include <boost/process.hpp>
#include <boost/regex.hpp>
#include <boost/asio.hpp>  
#include <boost/asio/steady_timer.hpp> 
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <securec.h>

#include "log/Log.h"
#include "system/basesystem/basesystem.hpp"
#ifndef __WINDOWS__ 
#include <sys/wait.h>
#include <sys/vfs.h>
#endif
#include <signal.h>
#include "config_reader/ConfigIniReader.h"

namespace bp = ::boost::process;

namespace Module {
const unsigned int STRING_ERROR_SIZE = 1024;
class AGENT_API SensitiveInfoWiper
{
public:
    SensitiveInfoWiper(const std::string& info): m_info(info)
    {
    }
    virtual ~SensitiveInfoWiper() = default;
    SensitiveInfoWiper& operator()(const std::string& pattern, const std::string& replace)
    {
#ifndef _DEBUG
        try
        {
            m_info = boost::regex_replace(m_info, boost::regex(pattern), replace);
        }
        catch (std::exception &e)
        {
            HCP_Logger_noid(ERR, "SecuritySS") << "incs/SecuritySS/system.hpp,get an exception: " << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
            return (*this);
        }
#else
        (void)pattern;
        (void)replace;
#endif
        return (*this);
    }
      const std::string& toString() const
     {
           return m_info;
     }
private:
    std::string m_info;
};

#define GET_LINES                                                                                                      \
    {                                                                                                                  \
        std::string line;                                                                                              \
        if (!bSec) {                                                                                                   \
            HCP_Logger(severity, moduleName, requestID)                                                                \
                << "stdout of @@@ " << wiper.toString() << " @@@" << HCPENDLOG;                                        \
        }                                                                                                              \
        if (!bSec) {                                                                                                   \
            HCP_Logger(severity, moduleName, requestID)                                                                \
                << "stderr of @@@ " << wiper.toString() << " @@@" << HCPENDLOG;                                        \
        }                                                                                                              \
        while (std::getline(is_stderr, line)) {                                                                        \
            HCP_Logger(ERR, moduleName, requestID) << "stderr: " << line << HCPENDLOG;                                 \
        }                                                                                                              \
        c.wait();                                                                                                      \
        if (!bSec) {                                                                                                   \
            if (c.exit_code() != 0) {                                                                                  \
                HCP_Logger(severity, moduleName, requestID)                                                            \
                    << "exit status of '" << wiper.toString() << "' was: " << c.exit_code() << HCPENDLOG;              \
            }                                                                                                          \
        }                                                                                                              \
        retv = c.exit_code();                                                                                          \
    }

    bool combineCmdstr(const severity_level& severity, const std::string& moduleName, const std::size_t& requestID,
        const std::string& inputcmd, std::vector<std::string> params, std::string& combineCmd);

    int RunLoggedSystemCommand(const severity_level& severity, const std::string& moduleName, const std::size_t& requestID,
        const std::string& cmd, std::vector<std::string> params, bool bSec = false);

    AGENT_API int runShellCmdWithOutput(const severity_level &severity, const std::string &moduleName,
        const std::size_t &requestID, const std::string &cmd, const std::vector <std::string> params,
        std::vector <std::string> &cmdoutput, std::vector <std::string> &stderroutput);

    std::string FormatFullUrl(const std::string& fullUrl);

    inline void hcp_sleep(const struct timespec timeout)
    {
        boost::this_thread::no_interruption_point::sleep_for(boost::chrono::seconds(timeout.tv_sec)
                                                             + boost::chrono::nanoseconds(timeout.tv_nsec));
        return;
    }

    inline void hcp_sleep(const long& sec)
    {
        struct timespec timeout = {time_t(sec), 0};
        hcp_sleep(timeout);

        return;
    }
    void killAfterSyncLog();
    void printCmdOutputInfo(const std::string& moduleName,
                            const std::size_t& requestID,
                            std::vector<std::string>& cmdoutput,
                            std::vector<std::string>& stderroutput);
    void getCurrentFilePath(boost::filesystem::path& fullFileName, const std::string& moduleName, const uint64_t& requestID);
    void getCurrentFilePath(std::string& fullFileName, const std::string& moduleName, const uint64_t& requestID);  //lint !e121
    inline std::string hcp_strerror(int errnum)
    {
        #ifndef __WINDOWS__
            char buf[STRING_ERROR_SIZE];
        #ifdef _AIX
            char* msg = __linux_strerror_r(errnum, buf, sizeof(buf));
        #elif defined (SOLARIS)
            char* msg = nullptr;
            int ret = strerror_r(errnum, buf, sizeof(buf));
            if (!ret) {
                msg = buf;
            }
        #else
            char* msg = strerror_r(errnum, buf, sizeof(buf));
        #endif  // _AIX
            if(NULL == msg)
            {
                return std::string("");
            }
            return std::string(msg);
        #else
            return "Not support to convent in windows";
        #endif
    }
#ifdef WIN32
    AGENT_API int ExecWinCmd(const std::string& cmd, uint32_t& retCode);
#endif
}
#endif

