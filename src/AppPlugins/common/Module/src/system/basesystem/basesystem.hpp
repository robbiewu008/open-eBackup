#ifndef BOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC
#define BOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC
#endif
#ifndef MODULE_BASE_SYSTEM_HPP
#define MODULE_BASE_SYSTEM_HPP
#define BOOST_POSIX_HAS_VFORK 1

#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#ifndef WIN32
#include <boost/process/posix.hpp>
#include <boost/process/pipe.hpp>
#endif
#include <boost/regex.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <boost/thread.hpp>
#include <sys/types.h>
#include <securec.h>

namespace bp = ::boost::process;

namespace Module {
    bp::child StartChild(
            const std::string &cmd, bp::ipstream &is_out, bp::ipstream &is_err, const unsigned int &runShellType);

    bool checkParam(std::vector<std::string> params);

    int BaseRunShellCmdWithOutputWithOutLock(const std::string &moduleName,
                                             const std::size_t &requestID,
                                             const std::string &cmd,
                                             const std::vector<std::string> params,
                                             std::vector<std::string> &cmdoutput,
                                             std::vector<std::string> &stderroutput,
                                             std::stringstream &outstring,
                                             const unsigned int &runShellType);

    int BaseRunShellCmdWithOutput(const std::string &moduleName,
                                  const std::size_t &requestID,
                                  const std::string &cmd,
                                  const std::vector<std::string> params,
                                  std::vector<std::string> &cmdoutput,
                                  std::vector<std::string> &stderroutput,
                                  std::stringstream &outstring,
                                  const unsigned int &runShellType);

}

#endif