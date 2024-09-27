#ifndef ICERTIFICATEHADNDLER_H_
#define ICERTIFICATEHADNDLER_H_
#include <string>
#include "servicecenter/certificateservice/include/ICertificateComm.h"

namespace certificateservice {
class ICertificateHandler {
public:
    ICertificateHandler() = default;
    virtual ~ICertificateHandler() = default;
    virtual std::string GetCertificateFile(CertificateType type) = 0;
    virtual bool GetCertificateConfig(CertificateConfig config, std::string& value) = 0;
};
}  // namespace certificateservice
#endif