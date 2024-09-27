#ifndef NGINX_CERTIFTCATE_PATH_PROXY_H_
#define NGINX_CERTIFTCATE_PATH_PROXY_H_
#include "servicecenter/certificateservice/include/ICertificatePathProxy.h"
#include "common/Defines.h"
#include <map>

namespace certificateservice {
namespace detail {
class NginxCertificatePathProxy : public ICertificatePathProxy {
public:
    NginxCertificatePathProxy() = default;
    virtual ~NginxCertificatePathProxy() = default;
    virtual std::string GetCertificateRootPath();
    virtual std::string GetCertificateFileName(CertificateType type);
    EXTER_ATTACK virtual bool GetCertificateConfig(CertificateConfig config, std::string& value);

private:
    bool GetPassword(std::string& pw);
    bool GetAlgorithmSuite(std::string& pw);
    mp_void GetHostName(std::string& pw);
    bool GetCertificateFileNameFromXml(CertificateType type, const std::string& config, std::string& value);

private:
    using FilePair = std::pair<std::string, std::string>;
    static std::map<CertificateType, FilePair> g_certificateFileName;
};
}
}
#endif