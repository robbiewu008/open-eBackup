#include <certificateservice/detail/CertificateHandler.h>
namespace certificateservice {
namespace detail {
std::string CertificateHandler::GetCertificateFile(CertificateType type)
{
    return m_pathProxy->GetCertificateRootPath() + "/" + m_pathProxy->GetCertificateFileName(type);
}

bool CertificateHandler::GetCertificateConfig(CertificateConfig config, std::string& value)
{
    return m_pathProxy->GetCertificateConfig(config, value);
}
}
}