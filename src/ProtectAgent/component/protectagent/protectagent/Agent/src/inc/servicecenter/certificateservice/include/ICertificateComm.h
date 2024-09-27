#ifndef ICERTIFICATECOMM_H_
#define ICERTIFICATECOMM_H_

namespace certificateservice {
enum class CertificateType {
    KEY_FILE,
    TRUSTE_CRETIFICATE_FILE,
    USE_CRETIFICATE_FILE
};

enum class CertificateConfig {
    PASSWORD,
    ALGORITEHM_SUITE,
    HOST_NAME
};
}

#endif