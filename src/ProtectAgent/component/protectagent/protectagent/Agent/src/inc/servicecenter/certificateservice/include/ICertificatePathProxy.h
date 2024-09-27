#ifndef ICERTIFICATE_PATHP_ROXY_H_
#define ICERTIFICATE_PATHP_ROXY_H_
#include <string>
#include "servicecenter/certificateservice/include/ICertificateComm.h"

namespace certificateservice {
class ICertificatePathProxy {
public:
    ICertificatePathProxy() = default;
    virtual ~ICertificatePathProxy() = default;
    virtual std::string GetCertificateRootPath() = 0;
    virtual std::string GetCertificateFileName(CertificateType type) = 0;
    virtual bool GetCertificateConfig(CertificateConfig config, std::string& value) = 0;
};
}  // namespace certificateservice

#endif
