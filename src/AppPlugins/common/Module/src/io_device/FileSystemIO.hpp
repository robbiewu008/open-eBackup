#ifndef FILESYSTEMIO_HPP
#define FILESYSTEMIO_HPP

#include <limits>
#include <stdio.h>
#include <stdint.h>
#ifdef _AIX
#define _LINUX_SOURCE_COMPAT
#endif
#include <string>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <fcntl.h>
#ifndef WIN32
#include <sys/statvfs.h>
#endif
#include <memory>
#include "IODeviceDef.h"
#include "define/Defines.h"
#include "define/Types.h"
#include "log/Log.h"

#include "IODeviceInterface.h"
#include "system/System.hpp"
#include "FileDef.h"

#include "common/Timer.h"
#ifdef __WINDOWS__
#include <io.h>
#define fsync     _commit
#define fileno    _fileno
#define ftruncate _chsize
#endif

static const int STRING_ERROR_SIZE = 1024;

namespace Module {
class FileSystemIO : public IODevice {
public:
	FileSystemIO () : m_fp(nullptr)
	{
        m_errno = 0;
        HCP_Log(DEBUG, IODeviceModule) << "Constructor."<<HCPENDLOG;
    }
	
    FileSystemIO(const IODeviceInfo &deviceInfo):m_fp(nullptr)
    {
        m_errno = 0;
        HCP_Log(DEBUG, IODeviceModule) << "Constructor." << HCPENDLOG;
    }

    virtual ~FileSystemIO()
    {
        HCP_Log(DEBUG, IODeviceModule) << "Destructor." << HCPENDLOG;
        if (m_fp != nullptr) {
            (void)Close();
        }

        m_fp = nullptr;
    }

    virtual std::string GetErrorStr(int err)
    {
        char buf[STRING_ERROR_SIZE];
        char* msg = nullptr;
#if defined(_AIX)
        msg = __linux_strerror_r(err, buf, sizeof(buf));
#elif defined(WIN32)
        msg == nullptr;
#elif defined(SOLARIS)
        int ret = strerror_r(err, buf, sizeof(buf));
        if (!ret) {
            msg = buf;
        }
#elif defined(WIN32)
        msg = nullptr;
#else
        msg = strerror_r(err, buf, sizeof(buf));
#endif
        if (msg == nullptr) {
            return std::string("");
        }

        return std::string(msg);
    }

    virtual std::string GetErrorInfo()
    {
        int err = m_errno;
        return GetErrorStr(err);
    }

    virtual bool GetNotExistError()
    {
        return (m_errno == ENOENT);
    }

    virtual void SetDek(const std::string &dek)
    {
    }

    virtual bool processAppend(EBKFileHandle *handle)
    {
        bool ret = true;
        if (m_mode == "a") {
            long fileOffset = 0;
            int fromWhere = SEEK_END;

            if (handle != nullptr && handle->fileOffset != 0) {
                fileOffset = handle->fileOffset;
                fromWhere = SEEK_SET;
            }

            if (Seek(fileOffset, fromWhere) != 0) {
                ret = false;
                HCP_Log(ERR, IODeviceModule) << "File seek failed.errno:" << errno << HCPENDLOG;
            }
        }

        return ret;
    }

    inline void mode_check(std::string mode, const char *filename)
    {
        if (("w" == mode) || ("a" == mode)) {
            HCP_Log(DEBUG, IODeviceModule) << "chmod file: " << filename << HCPENDLOG;
#ifndef WIN32
            int nRet = chmod(filename, S_IRUSR | S_IWUSR);
#else
            ERRLOG("WIN32 NOT IMPLMENTED");
			int nRet = 1;
#endif
            if (nRet != 0) {
                m_errno = errno;
                HCP_Log(ERR, IODeviceModule) << "chmod file " << filename << " failed, nRet=" << nRet << ", errno=" << errno
                                             << HCPENDLOG;
            }
        }
    }
    virtual bool Open(const char *filename, const char *mode, EBKFileHandle *handle = nullptr)
    {
        m_errno = 0;
        Timer timer;
#ifndef WIN32
        m_fp = fopen(filename, mode);

        m_fileName = filename;
        m_mode = mode;
        if (m_fp != nullptr) {
            HCP_Log(DEBUG, IODeviceModule) << "Open file: " << &m_fp << HCPENDLOG;
            if (!processAppend(handle)) {
                fclose(m_fp);
                m_fp = nullptr;
                return false;
            }


#ifdef NATIVE_PROFILE
            mode_check(m_mode, filename)
#endif

            return true;
        }
#endif

        if (ENOENT != errno || ("w" != m_mode && "a" != m_mode && "w+" != m_mode)) {
            m_errno = errno;
            HCP_Log(ERR, IODeviceModule) << "Open file " << filename << " failed,mode=" << mode << ",errno="
                                         << m_errno << ". fopen End, duration=" << timer.Duration() << HCPENDLOG;
            return false;
        }
        /* fix DTS2019030501389 add by c00377603 */
        boost::filesystem::path path = boost::filesystem::system_complete(filename);
        try {
            // mkdir
            boost::system::error_code ec;
            Timer timerMkdir;
            boost::filesystem::create_directories(path.parent_path(), ec);
            if (ec.value() != boost::system::errc::success) {
                HCP_Log(ERR, IODeviceModule) << "boost::filesystem::create_directories failed,error: " << ec.message()
                                             << " directoryName: " << path.parent_path().string() << HCPENDLOG;
                return false;
            }
            HCP_Log(WARN, IODeviceModule) << "mkdir " << path.parent_path().string() << ", duration=" <<
                                          timerMkdir.Duration() << HCPENDLOG;
        } catch (const boost::filesystem::filesystem_error &e) {
            HCP_Log(ERR, IODeviceModule) << "boost::filesystem::create_directories() exception: " << e.code().message() <<
                                         "path: "
                                         << path.parent_path().string() << HCPENDLOG;
            return false;
        }
        /* fix DTS2019030501389 end by c00377603 */
        // reopen
        Timer timerReopen;
#ifndef WIN32
        m_fp = fopen(filename, mode);
#else
        ERRLOG("WIN32 NOT IMPLMENTED");
#endif
        if (m_fp == nullptr) {
            m_errno = errno;
            HCP_Log(DEBUG, IODeviceModule) << "Open file: " << &m_fp
                                           << ". fopen End, file name=" << filename << ", duration=" << timerReopen.Duration() << HCPENDLOG;
            return false;
        }

        HCP_Log(DEBUG, IODeviceModule) << "Open file: " << &m_fp << HCPENDLOG;

        if (!processAppend(handle)) {
            fclose(m_fp);
            m_fp = nullptr;
            return false;
        }

#ifdef NATIVE_PROFILE
        mode_check(m_mode, filename)
#endif

        return true;
    }

    virtual bool Close(FileCloseStatus status = FILE_CLOSE_STATUS_FINISH) {
        HCP_Log(DEBUG, IODeviceModule) << "Enter Close:m_fp" << &m_fp << HCPENDLOG;
        bool ret = true;
        (void)Flush();

        if (m_fp != nullptr) {
            Timer timer;
#ifndef WIN32
            ret = (fclose(m_fp) == 0 && true == ret) ? true : false;
            HCP_Log(DEBUG, IODeviceModule) << "fclose End, file name=" << m_fileName << ", duration=" << timer.Duration() <<
                                           HCPENDLOG;
#else
            ERRLOG("WIN32 NOT IMPLMENTED");
#endif
        }

        m_fp = nullptr;
        return ret;
    }

    virtual size_t Write(const void *ptr, size_t size, size_t count) {
        HCP_Log(DEBUG, IODeviceModule) << "Enter Write:m_fp" << &m_fp << HCPENDLOG;
        Timer timer;
        size_t rc = (m_fp == nullptr) ? 0 : fwrite(ptr, size, count, m_fp);
        HCP_Log(DEBUG, IODeviceModule) << "fwrite End, file name=" << m_fileName << ", duration=" << timer.Duration() <<
                                       HCPENDLOG;
        return rc;
    }

    virtual size_t WriteFS(const void* ptr, size_t size, size_t count) {
	return (m_fp == nullptr) ? 0 : fwrite(ptr, size, count, m_fp);
    }

    virtual size_t Append(const void *ptr, size_t count) {
        HCP_Log(DEBUG, IODeviceModule) << "Enter Open:m_fp" << &m_fp << HCPENDLOG;
        if (m_fp == nullptr) {
            HCP_Log(ERR, IODeviceModule) << " file fd is null." << HCPENDLOG;
            return 0;
        }

        Timer timer;
        if (m_mode != "a") {
            HCP_Log(ERR, IODeviceModule) << "Open mode is incorrect.m_mode:" << m_mode << HCPENDLOG;
            return 0;
        }

        size_t rc = fwrite(ptr, 1, count, m_fp);
        HCP_Log(DEBUG, IODeviceModule) << "file Append End, file name=" << m_fileName << ", duration=" << timer.Duration()
                                       << HCPENDLOG;
        return rc;
    }

    virtual size_t Read(void *ptr, size_t size, size_t count) {
        m_errno = 0;
        HCP_Log(DEBUG, IODeviceModule) << "Enter Read:m_fp" << &m_fp << HCPENDLOG;
        if (m_fp == nullptr) {
            HCP_Log(ERR, IODeviceModule) << " file fd is null." << HCPENDLOG;
            return 0;
        } else {
            Timer timer;
            size_t rc = fread(ptr, size, count, m_fp);

            if (rc != count) {
                m_errno = errno;
                HCP_Log(DEBUG, IODeviceModule) << "Read failed,rc=" << rc << ", size=" << size
                                               << ", count=" << count << ",error=" << strerror(m_errno)
                                               << ", duration=" << timer.Duration() << HCPENDLOG;
            }
            HCP_Log(DEBUG, IODeviceModule) << "fread End, file name=" << m_fileName << ", duration=" << timer.Duration() <<
                                           HCPENDLOG;
            return rc;
        }
    }

    virtual size_t ReadFS(void *ptr, size_t size, size_t count) {
        m_errno = 0;
        HCP_Log(DEBUG, IODeviceModule) << "Enter Read:m_fp" << &m_fp << HCPENDLOG;
        if (m_fp == nullptr) {
            HCP_Log(ERR, IODeviceModule) << " file fd is null." << HCPENDLOG;
            return 0;
        } else {
            Timer timer;
            size_t rc = fread(ptr, size, count, m_fp);

            if (rc != count) {
                m_errno = errno;
                HCP_Log(DEBUG, IODeviceModule) << "Read failed,rc=" << rc << ", size=" << size
                                               << ", count=" << count << ",error=" << strerror(m_errno)
                                               << ", duration=" << timer.Duration() << HCPENDLOG;
            }
            HCP_Log(DEBUG, IODeviceModule) << "fread End, file name=" << m_fileName << ", duration=" << timer.Duration() <<
                                           HCPENDLOG;
            return rc;
        }
    }

    virtual bool UseVpp() {
        return false;
    }

    virtual int Seek(long int offset, int origin = SEEK_SET) {
        HCP_Log(DEBUG, IODeviceModule) << "offset=" << offset << ",addr=" << &m_fp << HCPENDLOG;
        return (m_fp == nullptr) ? -1 : fseek(m_fp, offset, origin);
    }

    virtual long int Tell() {
        return (m_fp == nullptr) ? -1 : ftell(m_fp);
    }

    virtual bool FileSize(const char *filename, uintmax_t &size) {
        if ((filename == nullptr) || (strlen(filename) == 0)) {
            if (m_fp == nullptr) {
                HCP_Log(ERR, IODeviceModule) << " file fd is null." << HCPENDLOG;
                return false;
            }
            long curpos = 0;
            curpos = ftell(m_fp);
            fseek(m_fp, 0L, SEEK_END);
            size = ftell(m_fp);
            fseek(m_fp, curpos, SEEK_SET);
            return true;
        }

        try {
            boost::system::error_code ec;
            Timer timer;
            size = boost::filesystem::file_size(filename, ec);
            HCP_Log(DEBUG, IODeviceModule) << "get file size End, file name=" << filename << ", duration=" << timer.Duration()
                                           << HCPENDLOG;
            if (ec.value() != boost::system::errc::success) {
                HCP_Log(ERR, IODeviceModule) << "boost::filesystem::FileSize() reported failure as: "
                                             << ec.message() << "file name: " << filename << ", duration=" << timer.Duration() << HCPENDLOG;
            }
            return true;
        } catch (std::exception &e) {
            HCP_Log(ERR, IODeviceModule) << " what=" << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
            size = 0;
            return false;
        }
    }

    virtual int Truncate(long int position) {
        return (m_fp == nullptr) ? -1 : ftruncate(fileno(m_fp), position);
    }

    virtual int Flush(bool sync = true) {
        if (m_fp == nullptr) {
            return -1;
        }

        Timer timer;
        if (fflush(m_fp) != 0) {
            std::string errStr = GetErrorStr(errno);
            HCP_Log(ERR, IODeviceModule) << " fflush failed, errno=" << errStr
                                         << ". file name=" << m_fileName << ", duration=" << timer.Duration() << HCPENDLOG;
            return -1;
        }

        Timer timerFsync;
        if (sync && (fsync(fileno(m_fp)) != 0)) {
            std::string errStr = GetErrorStr(errno);
            HCP_Log(ERR, IODeviceModule) << " fsync failed, errno=" << errStr
                                         << ". file name=" << m_fileName << ", duration=" << timerFsync.Duration() << HCPENDLOG;
            return -1;
        }
        HCP_Log(DEBUG, IODeviceModule) << "file flush End, file name=" << m_fileName << ", duration=" << timer.Duration()
                                       << ". fsync End, duration=" << timerFsync.Duration() << HCPENDLOG;

        return 0;
    }

    virtual bool Loaded() {
        return (nullptr != m_fp);
    }

    virtual bool Exists(const char *filename) {
        m_errno = 0 ;
        boost::system::error_code ec;
        bool exists = false;

        try {
            Timer timer;
            exists = boost::filesystem::exists(filename, ec);
            m_errno = errno;
            HCP_Log(DEBUG, IODeviceModule) << "check file exists End, file name=" << filename << ", duration=" <<
                                           timer.Duration() << HCPENDLOG;

            if (ec.value() != boost::system::errc::success) {
                HCP_Log(DEBUG, IODeviceModule) << "boost::filesystem::exists() reported failure as: "
                                               << ec.message() << ". filename: " << filename << ", duration=" << timer.Duration() << HCPENDLOG;
            }
        } catch (const boost::filesystem::filesystem_error &e) {
            HCP_Log(ERR,
                    IODeviceModule)
                    << "boost::filesystem::exists() exeption: " << e.code().message() << " path " << filename << HCPENDLOG;
        }
        return exists;
    }

    inline void get_suffixes(std::string directory, std::string prefixFile, std::vector<std::string> &suffixs) {
        for (boost::filesystem::directory_iterator it(directory.c_str()); it != boost::filesystem::directory_iterator();
             ++it) {
            std::string tempFile = it->path().string();
            std::string::size_type index = tempFile.find(prefixFile);
            if (std::string::npos != index) {
                std::string suffix(tempFile.begin() + index + prefixFile.size(), tempFile.end());
                suffixs.push_back(suffix);
            }
        }
    }

    virtual bool FilePrefixExists(const char *filePrefixName, std::vector<std::string> &suffixs) {
        if (filePrefixName == nullptr) {
            HCP_Log(ERR, IODeviceModule) << "param is error!" << HCPENDLOG;
            return false;
        }

        std::string path = filePrefixName;
        int pos = path.find_last_of('/');
        if (pos < 0) {
            HCP_Log(ERR, IODeviceModule) << "param is error!" << HCPENDLOG;
            return false;
        }
        std::string prefixFile(path.begin() + pos + 1, path.end());
        std::string directory(path.begin(), path.begin() + pos);
        if (directory.size() == 0 || prefixFile.size() == 0) {
            HCP_Log(ERR, IODeviceModule) << "param is error!" << HCPENDLOG;
            return false;
        }

        if (!CreateDirectory(directory.c_str())) {
            HCP_Log(ERR, IODeviceModule) << " create directory failed, directory is " << directory << HCPENDLOG;
            return false;
        }

        try {
            get_suffixes(directory, prefixFile, suffixs);
        } catch (std::exception &e) {
            HCP_Log(ERR, IODeviceModule) << " what=" << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
            return false;
        }

        HCP_Log(DEBUG, IODeviceModule) << "end FilePrefixExists success." << HCPENDLOG;
        return true;
    }

    virtual bool Remove(const char *filename) {
        if (filename == nullptr) {
            HCP_Log(WARN, IODeviceModule) << "param is error!" << HCPENDLOG;
            return false;
        }
        if (!Exists(filename)) {
            HCP_Log(WARN, IODeviceModule) << "removing file, but not exists. name: " << filename << HCPENDLOG;
            return true;
        }

        Timer timer;
        try {
            if (boost::filesystem::remove(filename) == false) {
                HCP_Log(ERR, IODeviceModule) << "failure when removing file. name: " << filename << HCPENDLOG;
                return false;
            }
        } catch (const boost::filesystem::filesystem_error &e) {
            HCP_Log(ERR,
                    IODeviceModule)
                    << "boost::filesystem::remove() catch exeption: " << e.code().message() << " path " << filename << HCPENDLOG;
            return false;
        }
        HCP_Log(DEBUG, IODeviceModule) << "file remove End, file name=" << filename << ", duration=" << timer.Duration()
                                       << HCPENDLOG;
        return true;
    }

    virtual bool RemoveObjs(const std::vector<std::string> &directoryNames, std::vector<std::string> &objs,
                            uintmax_t &size, const uint8_t &snapVersion) {
        Timer timer;
        std::string directoryName = directoryNames[0];

        for (const auto &obj : objs) {
            std::string file_name = directoryName + "/" + obj;
            uintmax_t size_temp = 0;
            if (true == FileSize(file_name.c_str(), size_temp) && snapVersion < 2) {
                size += size_temp;
            } else if (false == FileSize(file_name.c_str(), size_temp)) {
                HCP_Log(ERR, IODeviceModule) << "error when get file size" << HCPENDLOG;
                return false;
            }
            if (!Remove(file_name.c_str())) {
                return false;
            }
        }
        HCP_Log(DEBUG, IODeviceModule) << "file remove End, duration=" << timer.Duration() << HCPENDLOG;
        return true;
    }

    virtual bool RemoveAll(const char *directoryName) {
        if (!Exists(directoryName)) {
            HCP_Log(WARN, IODeviceModule) << "removing file, but not exists. name: " << directoryName << HCPENDLOG;
            return true;
        }

        std::vector<std::string> paramList;
        paramList.push_back(directoryName);
        std::string cmd = "rm -rf '?'";
        Timer timer;
        if (RunLoggedSystemCommand(DEBUG, IODeviceModule, 0, cmd, paramList) != 0) {
            HCP_Log(ERR, IODeviceModule) << "Force to delete dir failed. Delete dir path:" << directoryName << HCPENDLOG;

            return false;
        }
        HCP_Log(DEBUG, IODeviceModule) << "remove all files End, dir path=" << directoryName << ", duration=" <<
                                       timer.Duration() << HCPENDLOG;
        return true;
    }

    virtual bool Rename(const char *oldName, const char *newName) {
        if ((oldName == nullptr) || (newName == nullptr)) {
            HCP_Log(ERR, IODeviceModule) << "param invalid" << HCPENDLOG;
            return false;
        }

        if (!Exists(oldName)) {
            return false;
        }

        try {
            Timer timer;
            boost::filesystem::rename(oldName, newName);
            HCP_Log(DEBUG, IODeviceModule) << "file rename End, old name=" << oldName << ", duration=" << timer.Duration() <<
                                           HCPENDLOG;
            m_fileName = newName;

            return true;
        } catch (std::exception &e) {
            HCP_Log(ERR,
                    IODeviceModule)
                    << " Rename file error, what=" << WIPE_SENSITIVE(e.what())
                    << " from = " << oldName << " to " << newName << HCPENDLOG;
            return false;
        }
    }

    // virtual bool Permissions ();
    // virtual bool system_complete();

    virtual bool CreateDirectory(const char *directoryName) {
        HCP_Log(DEBUG, IODeviceModule) << "Entered CreateDirectory  " << HCPENDLOG;
        if (directoryName == nullptr) {
            HCP_Log(ERR, IODeviceModule) << "param invalid" << HCPENDLOG;
            return false;
        }

        boost::system::error_code ec;
        bool res = false;

        try {
            res = IsDirectory(directoryName);
            if (res) {
                HCP_Log(DEBUG, IODeviceModule) << "It is a directory. Directory name: " << directoryName << HCPENDLOG;
                return true;
            }
            HCP_Log(DEBUG, IODeviceModule) << "Not a directory, begin to create. Directory name: " << directoryName <<
                                           HCPENDLOG;

            res = boost::filesystem::create_directories(directoryName, ec);
            if (ec.value() != boost::system::errc::success) {
                HCP_Log(ERR, IODeviceModule) << "boost::filesystem::create_directories failed, error: " << ec.message()
                                             << " directory name: " << directoryName << HCPENDLOG;
                res = false;
            }

            boost::filesystem::permissions(directoryName, boost::filesystem::perms::owner_all, ec);
            if (ec.value() != boost::system::errc::success) {
                HCP_Log(ERR, IODeviceModule) << "boost::filesystem::permissions failed, error: " << ec.message()
                                             << " directory name: " << directoryName << HCPENDLOG;
                res = false;
            }
        } catch (const boost::filesystem::filesystem_error &e) {
            HCP_Log(ERR,
                    IODeviceModule)
                    << "boost::filesystem::create_directories() exeption: " << e.code().message() << " path "
                    << directoryName << HCPENDLOG;
        }
        HCP_Log(DEBUG, IODeviceModule) << "CreateDirectory return: " << res << HCPENDLOG;
        return res;
    }

    virtual bool CreateCifsDirectory(const char *directoryName)
    {
        HCP_Log(DEBUG, IODeviceModule) << "Entered CreateDirectory  " << HCPENDLOG;
        if (directoryName == nullptr) {
            HCP_Log(ERR, IODeviceModule) << "param invalid" << HCPENDLOG;
            return false;
        }

        boost::system::error_code ec;
        bool res = false;

        try {
            res = IsDirectory(directoryName);
            if (res) {
                HCP_Log(DEBUG, IODeviceModule)
                    << "The directory already exists. Directory name: " << directoryName << HCPENDLOG;
                return true;
            }
            HCP_Log(DEBUG, IODeviceModule)
                << "Not a directory, begin to create. Directory name: " << directoryName << HCPENDLOG;

            res = boost::filesystem::create_directories(directoryName, ec);
            if (ec.value() != boost::system::errc::success) {
                HCP_Log(ERR, IODeviceModule) << "boost::filesystem::create_directories failed, error: " << ec.message()
                                             << " directory name: " << directoryName << HCPENDLOG;
                res = false;
            }
        } catch (const boost::filesystem::filesystem_error &e) {
            HCP_Log(ERR, IODeviceModule) << "boost::filesystem::create_directories() exeption: " << e.code().message()
                                         << " path " << directoryName << HCPENDLOG;
            return false;
        }
        HCP_Log(DEBUG, IODeviceModule) << "CreateDirectory return: " << res << HCPENDLOG;
        return res;
    }

    virtual bool IsDirectory(const char *directoryName) {
        boost::system::error_code ec;
        bool res = false;

        try {
            res = boost::filesystem::is_directory(directoryName, ec);
            if (ec.value() != boost::system::errc::success) {
                HCP_Log(DEBUG, IODeviceModule) << "boost::filesystem::is_directory failed, error: " << ec.message()
                                             << " path name: " << directoryName << HCPENDLOG;
            }
        } catch (const boost::filesystem::filesystem_error &e) {
            HCP_Log(ERR,
                    IODeviceModule)
                    << "boost::filesystem::is_directory() exeption: " << e.code().message() << " path "
                    << directoryName << HCPENDLOG;
        }
        return res;
    }

    virtual void Reset(const char *fileName, const char *mode) {
        if (m_fp != nullptr) {
            if (!Close()) {
                int err = errno;
                HCP_Log(ERR, IODeviceModule) << " fclose failed, errno=" << strerror(err) << HCPENDLOG;
            }
        }

        (void)Open(fileName, mode);
    }

    virtual bool Copy(const char *srcName, const char *destName) {
        if ((srcName == nullptr) || (destName == nullptr)) {
            HCP_Log(ERR, IODeviceModule) << "param invalid" << HCPENDLOG;
            return false;
        }

        Timer timer;
        try {
            boost::filesystem::path fromPath(srcName);
            boost::filesystem::path toPath(destName);
            if (boost::filesystem::is_directory(destName)) {
                std::string fileName = fromPath.filename().string();
                size_t pos = fileName.find_last_of("/");
                if (pos != std::string::npos) {
                    fileName = fileName.substr(pos + 1);
                }
                toPath = toPath / fileName;
            }
            boost::filesystem::copy_file(fromPath, toPath, boost::filesystem::copy_option::overwrite_if_exists);
            HCP_Log(DEBUG, IODeviceModule) << "Succeeded in copying file from path: " << srcName
                                           << ", to path: " << destName << ", duration=" << timer.Duration() << HCPENDLOG;
        } catch (const boost::filesystem::filesystem_error &e) {
            if (e.code() == boost::system::errc::permission_denied) {
                HCP_Log(ERR, IODeviceModule)
                        << "boost filesystem copy_file() threw an exception as permission_denied for from_domainPath: "
                        << srcName << ", and to_path: " << destName << ", with error: " << e.code().message()
                        << ", duration=" << timer.Duration() << HCPENDLOG;
            } else {
                HCP_Log(ERR, IODeviceModule)
                        << "boost filesystem copy_file() threw an exception for from_domainPath: "
                        << srcName << ", and to_path: " << destName << ", with error: " << e.code().message()
                        << ", duration=" << timer.Duration() << HCPENDLOG;
            }
            return false;
        }
        return true;
    }

    virtual bool CopyDirectory(const char *srcName, const char *destName) {
        try {
            boost::filesystem::path src(srcName);
            boost::filesystem::path dst(destName);
            if (boost::filesystem::is_directory(src)) {
                std::string fileName = src.filename().string();
                size_t pos = fileName.find_last_of("/");
                if (pos != std::string::npos) {
                    fileName = fileName.substr(pos + 1);
                }
                dst = dst / fileName;
            }
            return CopyDirectory(src, dst);
        } catch (std::exception &e) {
            HCP_Log(ERR, "File") << " what=" << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
            return false;
        }
    }

    virtual bool GetDirectoryList(const char *directoryName, std::vector<std::string> &elementList) {
        if (directoryName == nullptr) {
            HCP_Log(ERR, IODeviceModule) << "param invalid" << HCPENDLOG;
            return false;
        }

        if (!Exists(directoryName)) {
            return false;
        }

        if (!IsDirectory(directoryName)) {
            return false;
        }

        try {
            for (boost::filesystem::directory_iterator it(directoryName);
                 it != boost::filesystem::directory_iterator(); ++it) {
                if (/* boost::filesystem::is_regular_file(*it) || */ boost::filesystem::is_directory(*it)) {
                    elementList.push_back(it->path().string());
                }
            }
        } catch (std::exception &e) {
            HCP_Log(ERR, IODeviceModule) << " what=" << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
            return false;
        }

        return true;
    }
    virtual bool GetFileListInDirectory(const char *directoryName, std::vector<std::string> &elementList) {
        if (directoryName == nullptr) {
            HCP_Log(ERR, IODeviceModule) << "param invalid" << HCPENDLOG;
            return false;
        }

        if (!Exists(directoryName)) {
            return false;
        }

        if (!IsDirectory(directoryName)) {
            return false;
        }

        try {
            for (boost::filesystem::directory_iterator it(directoryName);
                 it != boost::filesystem::directory_iterator(); ++it) {
                if (boost::filesystem::is_regular_file(*it)) {
                    elementList.push_back(it->path().string());
                }
            }
        } catch (std::exception &e) {
            HCP_Log(ERR, IODeviceModule) << " what=" << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
            return false;
        }

        return true;
    }

    virtual bool GetObjectListWithMarker(const std::string &dir, std::string &marker, bool &isEnd, int maxKeys,
        std::vector<std::string> &elementList)
    {
        return true;
    }

    virtual bool GetCommonPrefixList(const char *directoryName, const std::string &delimiter,
                                     std::vector<std::string> &elementList) {
        return true;
    }

    virtual bool TestConnect(const char *mountPath, const int retryTimes) {
        (void)retryTimes;
        if (mountPath  == nullptr) {
            HCP_Log(ERR, IODeviceModule) << "param invalid" << HCPENDLOG;
            return false;
        }
        std::vector<std::string> paramList;
        paramList.push_back(mountPath);
        std::string cmd = "LD_LIBRARY_PATH=\"\" mount | grep -E '?'";
        if (RunLoggedSystemCommand(DEBUG, IODeviceModule, 0, cmd, paramList) != 0) {
            // mount point do not exist.
            HCP_Log(WARN, IODeviceModule)
                << "boost::filesystem::exists() File or directory do not exist:" << mountPath << HCPENDLOG;
            return false;  // in case the file do not exist (legal in case of clean leftovers) it will return, res eq false, but this is legal
        }
        return true;
    }

    /****************************************************************************
    @ Description    : for backup storage is not S3. these functions are not used. 
                       The interfaces are used for only multi bucket for s3
    @ Begin
    *****************************************************************************/
    virtual bool TestSubBucketIsNotExisted(const char *bucketName, const int retryTimes) {
        return true;
    }

    virtual bool GetBucketObjNum(const char *bucketName, int64_t &objectNum) {
        return true;
    }
    /****************************************************************************
    @ Description    : for backup storage is not S3. these functions are not used. 
                       The interfaces are used for only multi bucket for s3
    @ End
    *****************************************************************************/

    virtual bool GetSpaceInfo(const char *pathName, uint64_t &capacity, uint64_t &free) {
#ifndef __WINDOWS__
        struct statvfs fsInfo;
        int rv = statvfs(pathName, &fsInfo);
        if (rv == 0) {
            capacity = fsInfo.f_frsize * (uint64_t)fsInfo.f_blocks;
            free = fsInfo.f_frsize * (uint64_t)fsInfo.f_bavail;
            HCP_Log(DEBUG, IODeviceModule) << "total : " << capacity << ", free:" << free
                                                                      << HCPENDLOG;
            return true;
        } else {
            int err = errno;
            HCP_Log(ERR, IODeviceModule) << "Get FS mount size [statvfs()] has errors, errno: " <<
                                                                    strerror(err) << HCPENDLOG;
            return false;
        }
#endif
        return false;
    }

    long int DirSize(const char *filename) {
        long length = 0;

        try {
            if ((filename == nullptr) || (strlen(filename) == 0)) {
                return 0;
            }
            Timer timer;
            for (boost::filesystem::directory_iterator it(filename); it != boost::filesystem::directory_iterator();
                 ++it) {
                if (boost::filesystem::is_regular_file(*it)) {
                    length += boost::filesystem::file_size(it->path());
                } else if (boost::filesystem::is_directory(*it)) {
                    length += DirSize(it->path().string().c_str());
                }
            }
            HCP_Log(DEBUG, IODeviceModule) << "get directory size End, dir name=" << filename << ", duration=" <<
                                           timer.Duration() << HCPENDLOG;
        } catch (std::exception &e) {
            HCP_Log(ERR, "File") << " what=" << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
        }
        return length;
    }

    boost::tribool FileExists(const char *filename) {
        return Exists(filename);
    }

    virtual int Lock(int op) {
        (void)op;
        return 0;
    }

    virtual int Unlock(int op) {
        (void)op;
        return 0;
    }

    virtual bool TestSubBucketExisted(const char *bucketName) {
        return false;
    }

    virtual bool TestBucketExisted(const char *bucketName) {
        return true;
    }

    virtual void RegisterCallbackHandle(CallBackHandle &handle)
    {
        return;
    }

    virtual void RegisterReadFileCallbackFun(const ReadFileCallback &handle)
    {
        return;
    }

    static std::shared_ptr<IODevice> CreateInstance(const IODeviceInfo &deviceInfo) {
        return std::shared_ptr<IODevice>(new (std::nothrow) FileSystemIO(deviceInfo));
    }

    virtual bool ReadNoCache(const char *filename, const std::string &dek, size_t offset, char *buffer,
                             const size_t bufferLen, size_t &readedLen) {
        m_errno = 0;
        if (filename == nullptr) {
            HCP_Log(ERR, IODeviceModule) << "File name is null." << HCPENDLOG;
            return false;
        }
        if (bufferLen == 0) {
            HCP_Log(ERR, IODeviceModule) << "Buffer len is 0." << HCPENDLOG;
            return false;
        }
        HCP_Log(DEBUG, IODeviceModule) << "fileName:" << filename
                                       << "offset: " << offset
                                       << ", buffer length: " << bufferLen << HCPENDLOG;
#ifndef WIN32
        FILE *fp = fopen(filename, "r");
#else
		FILE *fp = nullptr;
        ERRLOG("WIN32 NOT IMPLMENTED");
#endif
        if (fp == nullptr) {
            m_errno = errno;
            HCP_Log(ERR, IODeviceModule) << "Open file[" << filename << "] failed. errno: " << strerror(m_errno) << HCPENDLOG;
            return false;
        }
        int iRet = fseek(fp, offset, SEEK_SET);
        if (iRet != 0) {
            m_errno = errno;
            HCP_Log(ERR, IODeviceModule) << "seek file[" << filename
                                         << "] failed. errno: " << strerror(m_errno)
                                         << ", offset is " << offset << HCPENDLOG;
            fclose(fp);
            fp = nullptr;
            return false;
        }

        readedLen = fread(buffer, 1, bufferLen, fp);
        if (readedLen == 0) {
            m_errno = errno;
            HCP_Log(WARN, IODeviceModule) << "Read failed,rc=" << readedLen << ", size=" << bufferLen
                                          << ", count=" << 1 << ",error=" << strerror(m_errno) << HCPENDLOG;
        }
        fclose(fp);
        fp = nullptr;
        return true;
    }

    virtual bool DownloadFile(const std::string &objName, const std::string &destFile) {
        return true;
    }

    virtual bool NormalFileUpload(const std::string &localFile, const std::string &remoteFile) {
        return true;
    }

    virtual void SetCacheDataType(int cacheDataType) {
    }

    virtual int GetCacheDataType() {
        return 0;
    }

    virtual void SetUpLoadRateLimit(uint64_t qos) {
        return;
    }

    virtual void SetDownLoadRateLimit(uint64_t qos) {
        return;
    }

private:
    inline bool CopyDirectory(const boost::filesystem::path &src, const boost::filesystem::path &dst) {
        if (!boost::filesystem::exists(dst)) {
            /* fix DTS2019030501389 add by c00377603 */
            try {
                boost::system::error_code ec;
                boost::filesystem::create_directories(dst, ec);
                if (ec.value() != boost::system::errc::success) {
                    HCP_Log(ERR, IODeviceModule) << "boost::filesystem::create_directories failed,error: " << ec.message()
                                                 << "directoryName: " << dst.string() << HCPENDLOG;
                    return false;
                }
            } catch (const boost::filesystem::filesystem_error &e) {
                HCP_Log(ERR, IODeviceModule) << "boost::filesystem::create_directories() exception: " << e.code().message() <<
                                             "path: "
                                             << dst.string() << HCPENDLOG;
                return false;
            }
        }
        /* fix DTS2019030501389 end by c00377603 */
        for (boost::filesystem::directory_iterator it(src); it != boost::filesystem::directory_iterator(); ++it) {
            const boost::filesystem::path newSrc = it->path();
            std::string fileName = it->path().filename().string();
            size_t pos = fileName.find_last_of("/");
            if (pos != std::string::npos) {
                fileName = fileName.substr(pos + 1);
            }
            const boost::filesystem::path newDst = dst / fileName;
            Timer timer;
            if (boost::filesystem::is_directory(newSrc)) {
                CopyDirectory(newSrc, newDst);
                HCP_Log(DEBUG, IODeviceModule) << "CopyDirectory End, dir name=" << fileName << ", duration=" << timer.Duration()
                                               << HCPENDLOG;
            } else if (boost::filesystem::is_regular_file(newSrc)) {
                boost::filesystem::copy_file(newSrc, newDst, boost::filesystem::copy_option::overwrite_if_exists);
                HCP_Log(DEBUG, IODeviceModule) << "copy_file End, file name=" << fileName << ", duration=" << timer.Duration() <<
                                               HCPENDLOG;
            } else {
                HCP_Log(ERR, "File") << "Unrecognized file - " << newSrc
                                             << ". Copy file/dir End failed, duration=" << timer.Duration() << HCPENDLOG;
                return false;
            }
        }

        return true;
    }

private:
    FileSystemIO(const FileSystemIO &);
    const FileSystemIO &operator=(const FileSystemIO &);
    FILE *m_fp;
    std::string m_fileName;
    std::string m_mode;
    int m_errno;
};
} // namespace Module
#endif
