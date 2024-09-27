#ifndef CRETIFICATE_SERVICE_H_
#define CRETIFICATE_SERVICE_H_

#include "servicecenter/certificateservice/include/ICertificateService.h"

namespace certificateservice {
namespace detail {
class CertificateService : public ICertificateService {
public:
    CertificateService();
    virtual ~CertificateService();
    virtual bool Initailize();
    virtual bool Uninitailize();
    virtual std::shared_ptr<ICertificateHandler> GetCertificateHandler();
    // 通过代理的方式获取自定义的证书路径和证书名
    virtual std::shared_ptr<ICertificateHandler> GetCertificateHandler(
        const std::shared_ptr<ICertificatePathProxy>& pathProxy);

private:
    std::shared_ptr<ICertificatePathProxy> m_pathProxy;
};
}  // namespace detail
}  // namespace certificateservice
#endif