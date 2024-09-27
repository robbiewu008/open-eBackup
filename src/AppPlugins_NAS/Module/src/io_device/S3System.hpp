#ifndef S3SystemIO_H
#define S3SystemIO_H

#include <memory>
#include <boost/algorithm/string.hpp>
#include "IODeviceInterface.h"
#include "LibS3IO.h"
#include "IODeviceDef.h"
#include "log/Log.h"
#include "config_reader/ConfigIniReader.h"

namespace Module {
class S3SystemIO : public IODevice {
public:
    virtual std::string GetErrorInfo()
    {
        return m_lib3obj->GetErrorInfo();
    }

    virtual bool GetNotExistError()
    {
        return m_lib3obj->GetNotExistError();
    }

    virtual void SetDek(const std::string &dek)
    {
        m_lib3obj->SetDek(dek);
    }

    virtual bool Open(const char *filename, const char *mode, EBKFileHandle *handle)
    {
        return m_lib3obj->Open(filename, mode, handle);
    }

    virtual bool Close(FileCloseStatus status)
    {
        return m_lib3obj->Close(status);
    }

    virtual size_t Write(const void *ptr, size_t size, size_t count)
    {
        char *buffer = (char *)ptr;

        if ((size == 0) || (count == 0)) {
            return 0;
        }

        if (std::size_t(-1) / size < count) {
            return 0;
        }

        return m_lib3obj->Write(buffer, size * count) / size;
    }

    virtual size_t WriteFS(const void* ptr, size_t size, size_t count)
    {
        char* buffer = (char*)ptr;

        if ((size == 0) || (count == 0)) {
            return 0;
        }

        if (std::size_t(-1) / size < count) {
            return 0;
        }

        return m_lib3obj->WriteFS(buffer, size * count) / size;
    }

    virtual size_t Append(const void *ptr, size_t count)
    {
        char *buffer = (char *)ptr;

        return m_lib3obj->Append(buffer, count);
    }

    virtual size_t Read(void *ptr, size_t size, size_t count)
    {
        char *buffer = (char *)ptr;

        if ((size == 0) || (count == 0)) {
            return 0;
        }

        if (std::size_t(-1) / size < count) {
            return 0;
        }

        return m_lib3obj->Read(buffer, size * count) / size;
    }

    virtual size_t ReadFS(void *ptr, size_t size, size_t count)
    {
        char *buffer = (char *)ptr;

        if ((size == 0) || (count == 0)) {
            return 0;
        }

        if (std::size_t(-1) / size < count) {
            return 0;
        }

        return m_lib3obj->ReadFS(buffer, size * count) / size;
    }

    virtual int Seek(long int offset, int origin = SEEK_SET)
    {
        return m_lib3obj->Seek(offset, origin);
    }

    virtual long int Tell()
    {
        return m_lib3obj->Tell();
    }

    virtual bool FileSize(const char *filename, uintmax_t &size)
    {
        return m_lib3obj->FileSize(filename, size);
    }

    virtual int Truncate(long int position)
    {
        return m_lib3obj->Truncate(position);
    }

    virtual int Flush(bool sync = true)
    {
        return m_lib3obj->Flush(sync);
    }

    virtual bool Loaded()
    {
        return m_lib3obj->Loaded();
    }

    virtual bool UseVpp()
    {
        return m_lib3obj->UseVpp();
    }

    virtual bool Exists(const char *filename)
    {
        return m_lib3obj->Exists(filename);
    }

    virtual bool RemoveObjs(const std::vector<std::string> &directoryNames, std::vector<std::string> &objs,
                            uintmax_t &size, const uint8_t &snapVersion)
    {
        return m_lib3obj->RemoveObjs(directoryNames, objs, size, snapVersion);
    }

    virtual bool FilePrefixExists(const char *filePrefixName, std::vector<std::string> &suffixs)
    {
        return m_lib3obj->FilePrefixExists(filePrefixName, suffixs);
    }

    virtual bool Remove(const char *filename)
    {
        return m_lib3obj->Remove(filename);
    }

    virtual bool RemoveAll(const char *directoryName)
    {
        return m_lib3obj->RemoveAll(directoryName);
    }

    virtual bool Rename(const char *oldName, const char *newName)
    {
        return m_lib3obj->Rename(oldName, newName);
    }

    virtual bool CreateDirectory(const char *directoryName)
    {
        return m_lib3obj->CreateDirectory(directoryName);
    }

    virtual bool CreateCifsDirectory(const char *directoryName)
    {
        return m_lib3obj->CreateDirectory(directoryName);
    }

    virtual bool IsDirectory(const char *directoryName)
    {
        return m_lib3obj->IsDirectory(directoryName);
    }

    virtual void Reset(const char *filename, const char *mode)
    {
        m_lib3obj->Reset(filename, mode);
    }

    virtual bool Copy(const char *srcName, const char *destName)
    {
        return m_lib3obj->Copy(srcName, destName);
    }

    virtual bool GetDirectoryList(const char *directoryName, std::vector<std::string> &elementList)
    {
        return m_lib3obj->GetDirectoryList(directoryName, elementList);
    }

    virtual bool GetFileListInDirectory(const char *directoryName, std::vector<std::string> &elementList)
    {
        return m_lib3obj->GetFileListInDirectory(directoryName, elementList);
    }

    virtual bool GetObjectListWithMarker(const std::string &dir, std::string &marker, bool &isEnd, int maxKeys,
        std::vector<std::string> &elementList)
    {
        return m_lib3obj->GetObjectListWithMarker(dir, marker, isEnd, maxKeys, elementList);
    }

    virtual bool GetCommonPrefixList(const char *directoryName, const std::string &delimiter,
                                     std::vector<std::string> &elementList)
    {
        return m_lib3obj->GetCommonPrefixList(directoryName, delimiter, elementList);
    }

    virtual bool TestConnect(const char *bucketName, const int retryTimes)
    {
        return m_lib3obj->TestConnect(bucketName, retryTimes);
    }

    virtual bool TestSubBucketIsNotExisted(const char *bucketName, const int retryTimes)
    {
        return m_lib3obj->TestSubBucketIsNotExisted(bucketName, retryTimes);
    }

    virtual bool GetBucketObjNum(const char *bucketName, int64_t &objectNum)
    {
        return m_lib3obj->GetBucketObjNum(bucketName, objectNum);
    }

    inline long int DirSize(const char *filename)
    {
        return m_lib3obj->DirSize(filename);
    }

    inline boost::tribool FileExists(const char *filename)
    {
        return m_lib3obj->FileExists(filename);
    }

    static std::shared_ptr<IODevice> CreateInstance(const IODeviceInfo &deviceInfo, OBJECT_TYPE fileType)
    {
        std::string bucketFullName = deviceInfo.path_prefix;
        std::string keyStr(":/");
        if (std::string::npos ==bucketFullName.find(keyStr)) {
            HCP_Log(ERR, IODeviceModule) << "Path string split failed by bucketFullName." << HCPENDLOG;
            return nullptr;
        }
        std::shared_ptr<IODevice> spDevice;
        try {
            spDevice.reset(new S3SystemIO(deviceInfo, fileType));
        } catch (const std::exception &e) {
            HCP_Log(ERR, IODeviceModule) << WIPE_SENSITIVE(e.what()) << HCPENDLOG;
            spDevice.reset();
        }
        return spDevice;
    }

    virtual bool GetSpaceInfo(const char *pathName, uint64_t &capacity, uint64_t &free)
    {
        HCP_Log(DEBUG, IODeviceModule) << "get  space info. path is " << pathName << HCPENDLOG;
        bool bRet = m_lib3obj->GetCapacity(capacity);
        if (!bRet) {
            HCP_Log(ERR, IODeviceModule) << "get capacity failed. path is " << pathName << HCPENDLOG;
            return false;
        }

        uint64_t used = 0;
        bRet = m_lib3obj->GetUsed(used);
        if (!bRet) {
            HCP_Log(ERR, IODeviceModule) << "get used space failed. path is " << pathName << HCPENDLOG;
            return false;
        }

        free = 0;
        if (capacity > used) {
            free = capacity - used;
        }

        return true;
    }
    void cleanS3IOPasswd(std::string &pwd)
    {
        for (std::string::size_type i = 0; i < pwd.size(); ++i) {
            pwd[i] = (char)0xcc;
        }
    }

    std::string FormatConnectUrl(bool isHttps, const std::string &connectUrl)
    {
        uint32_t obsIpv6PortHttp = ConfigReader::getInt("General", "OBSS3ipv6PortHttp");
        uint32_t obsIpv6PortHttps = ConfigReader::getInt("General", "OBSS3ipv6PortHttps");
        HCP_Log(DEBUG, IODeviceModule) << "OBSS3ipv6PortHttp is " << obsIpv6PortHttp
                                       << " ,OBSS3ipv6PortHttps is " << obsIpv6PortHttps << HCPENDLOG;

        std::string portstr = "";
        if (isHttps) {
            portstr = std::to_string(obsIpv6PortHttps);
        } else {
            portstr = std::to_string(obsIpv6PortHttp);
        }

        uint32_t count = 0;
        for (uint32_t i = 0; i < connectUrl.length(); i++) {
            if (connectUrl[i] == ':') {
                count++;
            }
            if (count >= 2) {
                break;
            }
        }

        if (count <= 1) {
            // ipv4
            return connectUrl;
        } else {
            // ipv6 add 5080 port or 5443 port
            std::string ipv6str = connectUrl + ":" + portstr;
            return connectUrl;
        }
    }

    S3SystemIO(const IODeviceInfo &deviceInfo, OBJECT_TYPE fileType)
    {
        HCP_Log(DEBUG, IODeviceModule) << "enter S3SystemIO Constructor." << HCPENDLOG;
        S3IOParams params;
        params.bucketFullName = deviceInfo.path_prefix;
        params.passWord = deviceInfo.password;
        params.userName = deviceInfo.user_name;
        params.protocol = deviceInfo.using_https ? OBS_PROTOCOL_HTTPS : OBS_PROTOCOL_HTTP;
        params.uriStyle = deviceInfo.style;
        HCP_Log(DEBUG, IODeviceModule) << "The style is " << params.uriStyle << HCPENDLOG;
        std::string keyStr(":/");
        std::string::size_type pos =  params.bucketFullName.find(keyStr);
        params.host = params.bucketFullName.substr(0, pos);
        bool isHttps = (deviceInfo.using_https ? true : false);
        params.host = FormatConnectUrl(isHttps, params.host); 

        params.bucket = params.bucketFullName.substr(pos + keyStr.size());
        params.cert = deviceInfo.cert;

        params.HttpProxyInfo = deviceInfo.HttpProxyInfo;
        params.SpeedUpInfo = deviceInfo.SpeedUpInfo;

        m_lib3obj.reset(new libs3IO(params, fileType));
        cleanS3IOPasswd(params.passWord);
    }

    virtual bool CopyDirectory(const char *srcName, const char *destName)
    {
        HCP_Log(ERR, IODeviceModule) << "Unable to support CopyDirectory " << HCPENDLOG;
        return false;
    }
    virtual ~S3SystemIO()
    {
        HCP_Log(DEBUG, IODeviceModule) << "Destructor." << HCPENDLOG;
    }

    virtual bool ReadNoCache(const char *filename, const std::string &dek, size_t offset, char *buffer,
                             const size_t bufferLen, size_t &readedLen)
    {
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
        return m_lib3obj->ReadNoCache(filename, dek, offset, buffer, bufferLen, readedLen);
    }

    virtual bool DownloadFile(const std::string &objName, const std::string &destFile)
    {
        return m_lib3obj->DownloadFile(objName, destFile);
    }

    virtual bool NormalFileUpload(const std::string &localFile, const std::string &remoteFile)
    {
        return m_lib3obj->UploadFile(localFile, remoteFile);
    }

    virtual void SetCacheDataType(int cacheDataType)
    {
        return m_lib3obj->SetCacheDataType(cacheDataType);
    }

    virtual int GetCacheDataType()
    {
        return m_lib3obj->GetCacheDataType();
    }

    virtual bool TestSubBucketExisted(const char *bucketName)
    {
        return m_lib3obj->TestSubBucketExisted(bucketName);
    }

    virtual bool TestBucketExisted(const char *bucketName)
    {
        return m_lib3obj->TestBucketExisted(bucketName);
    }

    virtual void RegisterCallbackHandle(CallBackHandle &handle)
    {
        m_lib3obj->RegisterCallbackHandle(handle);
    }

    virtual void RegisterReadFileCallbackFun(const ReadFileCallback &handle)
    {
        m_lib3obj->RegisterReadFileCallbackFun(handle);
    }

    virtual void SetUpLoadRateLimit(uint64_t qos) {
        m_lib3obj->SetUpLoadRateLimit(qos);
    }

    virtual void SetDownLoadRateLimit(uint64_t qos) {
        m_lib3obj->SetDownLoadRateLimit(qos);
    }
private:
    S3SystemIO(const S3SystemIO &);
    const S3SystemIO &operator=(const S3SystemIO &);

private:
    std::shared_ptr<libs3IO> m_lib3obj;
};
} // namespace Module
#endif
