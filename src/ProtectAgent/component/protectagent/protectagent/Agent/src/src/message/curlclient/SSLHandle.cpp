#ifndef WIN32

#include "common/Log.h"
#include "common/Defines.h"
#include "openssl/err.h"
#include "openssl/x509v3.h"
#include "openssl/rsa.h"
#include "openssl/rand.h"
#include "openssl/engine.h"
#include "openssl/pem.h"

#include "openssl/evp.h"
#include "openssl/x509.h"
#include "message/curlclient/SSLHandle.h"

using namespace std;

namespace {
const mp_string NONE_CERT_STRING = "none";
const mp_string AUTO_MATCH_CERT_STRING = "auto_match";
const mp_string FORCE_AUTO_MATCH_CERT_STRING = "force_auto_match";
const mp_string INVALID_CERT_STRING = "invalid_cert";

const mp_char CERT_HANDLE_NUM_2 = 2;
const mp_char CERT_HANDLE_NUM_3 = 3;
const mp_char CERT_HANDLE_NUM_4 = 4;
const mp_char CERT_HANDLE_NUM_5 = 5;
const mp_char CERT_HANDLE_NUM_6 = 6;
const mp_char CERT_HANDLE_NUM_7 = 7;
const mp_char CERT_HANDLE_NUM_8 = 8;
const mp_char CERT_HANDLE_NUM_9 = 9;
const mp_char CERT_HANDLE_NUM_10 = 10;
const mp_char CERT_HANDLE_NUM_11 = 11;
const mp_char CERT_HANDLE_NUM_12 = 12;
const mp_char CERT_HANDLE_NUM_13 = 13;
const mp_char CERT_HANDLE_NUM_14 = 14;
const mp_char CERT_HANDLE_NUM_15 = 15;
const mp_char CERT_HANDLE_NUM_24 = 24;
const mp_char CERT_HANDLE_NUM_50 = 50;
const mp_char CERT_HANDLE_NUM_60 = 60;
const mp_char CERT_HANDLE_NUM_70 = 70;
const mp_char CERT_HANDLE_NUM_100 = 100;
const mp_int32 CERT_HANDLE_NUM_1000 = 1000;
const mp_int32 CERT_HANDLE_NUM_1900 = 1900;
const mp_int32 RSA_HARD_BIT = 2048;
const mp_int32 DSA_HARD_BIT = 2048;
const mp_int32 EC_HARD_BIT = 256;
const mp_int32 KU_KEY_CERT_BIT_8 = 8;
}  // namespace

static pthread_mutex_t *g_opensslLockCS = nullptr;

namespace {
void PthreadsThreadId(CRYPTO_THREADID *tid)
{
    CRYPTO_THREADID_set_numeric(tid, (unsigned long)pthread_self());
}

void PthreadsLockingCallback(mp_int32 mode, mp_int32 type, char *file, mp_int32 line)
{
    if (g_opensslLockCS == nullptr) {
        return;
    }
    if (mode & CRYPTO_LOCK) {
        (void)pthread_mutex_lock(&(g_opensslLockCS[type]));
    } else {
        (void)pthread_mutex_unlock(&(g_opensslLockCS[type]));
    }
}

void OpensslCryptoThreadSetup(void)
{
    mp_int32 i;
    g_opensslLockCS = static_cast<pthread_mutex_t *>(
        OPENSSL_malloc(CRYPTO_num_locks() * (mp_int32)sizeof(pthread_mutex_t)));
    if (!g_opensslLockCS) {
        /* Nothing we can do about this...void function! */
        return;
    }

    for (i = 0; i < CRYPTO_num_locks(); i++) {
        (void)pthread_mutex_init(&(g_opensslLockCS[i]), NULL);
    }

    CRYPTO_THREADID_set_callback(PthreadsThreadId);
    CRYPTO_set_locking_callback((void (*)(mp_int32, mp_int32, const char *, mp_int32))PthreadsLockingCallback);
}

void BioFreeAll(BIO* b)
{
    if (b != nullptr) {
        BIO_free_all(b);
    }
}

void SslCtxFree(SSL_CTX* p)
{
    if (p != nullptr) {
        SSL_CTX_free(p);
    }
}

void SkX509InfoPopFree(STACK_OF(X509_INFO)* p)
{
    if (p != nullptr) {
        sk_X509_INFO_pop_free(p, X509_INFO_free);
    }
}

void BioFree(BIO* pBioPem)
{
    if (pBioPem != nullptr) {
        BIO_free(pBioPem);
    }
}

void SkX509PopFree(STACK_OF(X509)* pCerts)
{
    if (pCerts != nullptr) {
        sk_X509_pop_free(pCerts, X509_free);
        pCerts = nullptr;
    }
}

void SetKeyUsage(ASN1_BIT_STRING& bitStr, unsigned short& keyusage)
{
    if (bitStr.length > 0) {
        keyusage = bitStr.data[0];
    }

    if (bitStr.length > 1) {
        keyusage = keyusage | static_cast <unsigned short>(bitStr.data[1] << KU_KEY_CERT_BIT_8);
    }
}

SecuritySS_RC ReturnForCheckCert(STACK_OF(X509)* pCerts, SecuritySS_RC rc)
{
    SkX509PopFree(pCerts);
    return rc;
}

SecuritySS_RC ReturnForConvToPEMCert(STACK_OF(X509)* pCerts, BIO* pBioPem, SecuritySS_RC rc)
{
    SkX509PopFree(pCerts);
    BioFree(pBioPem);
    return rc;
}

SecuritySS_RC ReturnForConvToPemcert2(BIO* pBioPem, SecuritySS_RC rc)
{
    BioFree(pBioPem);
    return rc;
}

CURLcode ReturnForSslCtxFunction(STACK_OF(X509)* pCerts, CURLcode rc)
{
    SkX509PopFree(pCerts);
    return rc;
}

SecuritySS_RC ReturnForFGetThumbprint(STACK_OF(X509)* pCerts, SecuritySS_RC rc)
{
    SkX509PopFree(pCerts);
    return rc;
}

int SSLHandleVerifyCallback(mp_int32 ok, X509_STORE_CTX *ctx)
{
    int err;
    int depth;
    err = X509_STORE_CTX_get_error(ctx);
    depth = X509_STORE_CTX_get_error_depth(ctx);
    X509 *errCert = X509_STORE_CTX_get_current_cert(ctx);

    if (!ok) {
        const char *msg = X509_verify_cert_error_string(err);
        if (msg != nullptr) {
            COMMLOG(OS_LOG_ERROR, "Error when verify cert depth: %d, error num: %d, Msg: %s", depth, err, msg);
        } else {
            COMMLOG(OS_LOG_ERROR, "Error when verify cert depth: %d, error num: %d.", depth, err);
        }

        BIO *bioErr = BIO_new(BIO_s_mem());
        X509_NAME_print_ex(bioErr, X509_get_subject_name(errCert), 0, XN_FLAG_ONELINE);
        char *p = nullptr;
        mp_void *pp = static_cast<mp_void*>(&p);
        long l = BIO_get_mem_data(bioErr, &pp);
        if (l > 0) {
            COMMLOG(OS_LOG_ERROR, "Err: %s.", p);
        }
        BioFree(bioErr);
    }

    return ok;
}
} // namespace

void SSLHandle::InitOpenSSL()
{
    // Openssl initialization
    OpensslCryptoThreadSetup();
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
}

void SSLHandle::CleanupOpenSSL()
{
    // Openssl cleanup
    CRYPTO_cleanup_all_ex_data();
    RAND_cleanup();
    EVP_cleanup();
    ERR_free_strings();
    ERR_remove_state(0);
}

SecuritySS_RC SSLHandle::CheckCert(const string &certText)
{
    STACK_OF(X509) * pCerts = sk_X509_new_null();
    if (pCerts == NULL) {
        return SecuritySS_InnerWrong;
    }

    if (certText == NONE_CERT_STRING) {
        COMMLOG(OS_LOG_INFO, "The value indicate delete the cert.");
        return ReturnForCheckCert(pCerts, SecuritySS_Success);
    }

    SecuritySS_RC rc = ConvToX509(certText, pCerts);
    if (rc != SecuritySS_Success) {
        COMMLOG(OS_LOG_ERROR, "ConvToX509 fail.");
        return ReturnForCheckCert(pCerts, rc);
    }

    X509 *pCert = NULL;
    for (mp_int32 i = 0; i < sk_X509_num(pCerts); i++) {
        pCert = sk_X509_value(pCerts, i);
        if (pCert == NULL) {
            COMMLOG(OS_LOG_ERROR, "pCert is NULL.");
            return ReturnForCheckCert(pCerts, SecuritySS_InnerWrong);
        }

        rc = CheckCertExpire(pCert);
        if (rc != SecuritySS_Success) {
            COMMLOG(OS_LOG_ERROR, "Cert is expired or wrong.");
            return ReturnForCheckCert(pCerts, rc);
        }

        rc = checkCertCipher(pCert);
        if (rc != SecuritySS_Success) {
            COMMLOG(OS_LOG_ERROR, "Public key use not hard algorithm or cert wrong.");
            return ReturnForCheckCert(pCerts, rc);
        }
    }
    return ReturnForCheckCert(pCerts, SecuritySS_Success);
}

SecuritySS_RC SSLHandle::ConvToPEMCert(const string &certText, string &pemCert)
{
    pemCert.clear();

    STACK_OF(X509) * pCerts = sk_X509_new_null();
    if (pCerts == nullptr) {
        return Security_Cert_Invalid;
    }

    if (certText == NONE_CERT_STRING) {
        COMMLOG(OS_LOG_WARN, "The value indicate delete the cert.");
        pemCert = certText;
        return ReturnForCheckCert(pCerts, SecuritySS_Success);
    }

    SecuritySS_RC rc = ConvToX509(certText, pCerts);
    if (rc != SecuritySS_Success) {
        COMMLOG(OS_LOG_ERROR, "ConvToX509 fail.");
        return ReturnForCheckCert(pCerts, rc);
    }

    if (sk_X509_num(pCerts) < 1) {
        COMMLOG(OS_LOG_ERROR, "No x509 certs found.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    X509 *pCert = nullptr;
    BIO *pPemBio = BIO_new(BIO_s_mem());
    if (pPemBio == nullptr) {
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    for (mp_int32 i = 0; i < sk_X509_num(pCerts); ++i) {
        pCert = sk_X509_value(pCerts, i);
        if (!PEM_write_bio_X509(pPemBio, pCert)) {
            COMMLOG(OS_LOG_ERROR, "PEM_write_bio_X509 fail, Security_Cert_Invalid.");
            return ReturnForConvToPEMCert(pCerts, pPemBio, Security_Cert_Invalid);
        }
    }

    BUF_MEM *mem = nullptr;
    BIO_get_mem_ptr(pPemBio, &mem);
    if (!mem || !mem->data || !mem->length) {
        COMMLOG(OS_LOG_ERROR, "BIO_get_mem_ptr fail, Security_Cert_Invalid.");
        return ReturnForConvToPEMCert(pCerts, pPemBio, Security_Cert_Invalid);
    }
    pemCert = string(mem->data, mem->length);
    return ReturnForConvToPEMCert(pCerts, pPemBio, SecuritySS_Success);
}

void SSLHandle::EnrichSSL(CURL *curlCtx, const string &certText)
{
    curl_easy_setopt(curlCtx, CURLOPT_SSLCERTTYPE, "PEM");

    if (certText == NONE_CERT_STRING
        /* BEGIN: Added by yulei, 2016/05/30    PN: for CERT module */
        || certText == AUTO_MATCH_CERT_STRING || certText == FORCE_AUTO_MATCH_CERT_STRING ||
        certText == INVALID_CERT_STRING || certText == ""
        /* END: Added by yulei, 2016/05/30 */
    ) {
        COMMLOG(OS_LOG_DEBUG, "Establish SSL without cert: %s.", certText.c_str());
        curl_easy_setopt(curlCtx, CURLOPT_SSL_VERIFYPEER, 0L);
    } else {
        COMMLOG(OS_LOG_DEBUG, "Establish SSL with cert.");
        curl_easy_setopt(curlCtx, CURLOPT_SSL_VERIFYPEER, 1L);

        curl_easy_setopt(curlCtx, CURLOPT_SSL_CTX_DATA, &certText);
        curl_easy_setopt(curlCtx, CURLOPT_SSL_CTX_FUNCTION, *(SSLHandle::SslctxFunction));
    }

    curl_easy_setopt(curlCtx, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curlCtx, CURLOPT_CAINFO, nullptr);
    curl_easy_setopt(curlCtx, CURLOPT_CAPATH, nullptr);
}

SecuritySS_RC SSLHandle::EnrichSSL(SSL_CTX *sslCtx, const string &certText)
{
    if (CURLE_OK != SslctxFunction(sslCtx, certText)) {
        COMMLOG(OS_LOG_ERROR, "Set caCert failed !.");
        return Security_Cert_Invalid;
    }

    return SecuritySS_Success;
}

SecuritySS_RC SSLHandle::CheckCertExpire(X509 *pCert)
{
    if (pCert == nullptr) {
        COMMLOG(OS_LOG_ERROR, "pCert is NULL.");
        return Security_Cert_Invalid;
    }

    asn1_string_st *before = X509_get_notBefore(pCert);
    if (!before) {
        COMMLOG(OS_LOG_ERROR, "X509_get_notBefore fail, Security_Cert_Invalid.");
        return Security_Cert_Invalid;
    }

    asn1_string_st *after = X509_get_notAfter(pCert);
    if (!after) {
        COMMLOG(OS_LOG_ERROR, "X509_get_notAfter fail, Security_Cert_Invalid.");
        return Security_Cert_Invalid;
    }

    ASN1_UTCTIME *pBeTime = ASN1_STRING_dup(before);
    if (!pBeTime) {
        COMMLOG(OS_LOG_ERROR, "ASN1_STRING_dup for begin time fail, Security_Cert_Invalid.");
        return Security_Cert_Invalid;
    }

    ASN1_UTCTIME *pAfTime = ASN1_STRING_dup(after);
    if (!pAfTime) {
        COMMLOG(OS_LOG_ERROR, "ASN1_STRING_dup for end time fail, Security_Cert_Invalid.");
        ASN1_UTCTIME_free(pBeTime);
        return Security_Cert_Invalid;
    }

    SecuritySS_RC rv = SecuritySS_Success;
    time_t timeNotBefore = ASN1_TIME_ConvertToTimeT(*pBeTime);
    time_t timeNotAfter = ASN1_TIME_ConvertToTimeT(*pAfTime);
    time_t currentTime = time(&currentTime);
    if (currentTime < timeNotBefore || currentTime > timeNotAfter) {
        COMMLOG(OS_LOG_ERROR, "Security_Cert_Expire.");
        rv = Security_Cert_Expire;
    }

    ASN1_UTCTIME_free(pBeTime);
    ASN1_UTCTIME_free(pAfTime);

    return rv;
}

SecuritySS_RC SSLHandle::GetCertExpireTime(const string &certText, time_t &ExpireTime)
{
    if (NONE_CERT_STRING == certText) {
        COMMLOG(OS_LOG_WARN, "The value indicate delete the cert.");
        return SecuritySS_Success;
    }

    STACK_OF(X509) * pCerts = sk_X509_new_null();
    if (pCerts == NULL) {
        return SecuritySS_InnerWrong;
    }

    SecuritySS_RC rc = ConvToX509(certText, pCerts);
    if (rc != SecuritySS_Success) {
        COMMLOG(OS_LOG_ERROR, "ConvToX509 fail, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, rc);
    }

    X509 *pCert;
    if (sk_X509_num(pCerts) < 1) {
        COMMLOG(OS_LOG_ERROR, "sk_X509_num less than 1, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    pCert = sk_X509_value(pCerts, 0);
    if (pCert == nullptr) {
        COMMLOG(OS_LOG_ERROR, "pCert is NULL, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    asn1_string_st *after = X509_get_notAfter(pCert);
    if (!after) {
        COMMLOG(OS_LOG_ERROR, "X509_get_notAfter fail, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    ASN1_UTCTIME *pAfTime = ASN1_STRING_dup(after);
    if (!pAfTime) {
        COMMLOG(OS_LOG_ERROR, "ASN1_STRING_dup for end time fail, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    ExpireTime = ASN1_TIME_ConvertToTimeT(*pAfTime);

    ASN1_UTCTIME_free(pAfTime);
    return ReturnForCheckCert(pCerts, SecuritySS_Success);
}

SecuritySS_RC SSLHandle::checkCertCipher(X509 *pCert)
{
    // EVP_PKEY的具体定义在include/crypto/evp.h中
    // X509的具体定义在include/crypto/x509.h中
    // 这两个结构体的别名定义都在openssl/ossl_type.h
    EVP_PKEY *pkey = X509_get0_pubkey(pCert);
    if (pkey == nullptr) {
        COMMLOG(OS_LOG_ERROR, "X509_get_pubkey fail, Security_Cert_Invalid.");
        return Security_Cert_Invalid;
    }

    // type的具体值在openssl/evp.h中
    switch (EVP_PKEY_get_id(pkey)) {
        case EVP_PKEY_RSA:
            if (EVP_PKEY_bits(pkey) < RSA_HARD_BIT) {
                COMMLOG(OS_LOG_ERROR, "Security_Cert_KeyAlgorithmNotSecurit.");
                return Security_Cert_KeyAlgorithmNotSecurit;
            }
            break;
        case EVP_PKEY_DSA:
            if (EVP_PKEY_bits(pkey) < DSA_HARD_BIT) {
                COMMLOG(OS_LOG_ERROR, "Security_Cert_KeyAlgorithmNotSecurit.");
                return Security_Cert_KeyAlgorithmNotSecurit;
            }
            break;
        case EVP_PKEY_EC:
            if (EVP_PKEY_bits(pkey) < EC_HARD_BIT) {
                COMMLOG(OS_LOG_ERROR, "Security_Cert_KeyAlgorithmNotSecurit.");
                return Security_Cert_KeyAlgorithmNotSecurit;
            }
            break;
        default:
            EVP_PKEY_free(pkey);
            return Security_Cert_KeyAlgorithmNotSecurit;
    }
    EVP_PKEY_free(pkey);
    return SecuritySS_Success;
}

SecuritySS_RC SSLHandle::ConvToX509(const mp_string &certText, STACK_OF(X509) * &pCerts)
{
    if (pCerts == nullptr) {
        return Security_Cert_Invalid;
    }

    if (certText.empty()) {
        return Security_Cert_Invalid;
    }

    // BIO is changed in PEM_read_bio_X509. So, need two BIO
    BIO *pBioPem;
    STACK_OF(X509_INFO) * pInfo;
    pBioPem = BIO_new_mem_buf(
        reinterpret_cast<void *>(const_cast<char *>(certText.c_str())), static_cast<mp_int32>(certText.length()));
    if (!pBioPem) {
        COMMLOG(OS_LOG_ERROR, "BIO_new_mem_buf fail for pem, Security_Cert_Invalid.");
        return ReturnForConvToPemcert2(pBioPem, Security_Cert_Invalid);
    }

    pInfo = PEM_X509_INFO_read_bio(pBioPem, NULL, NULL, NULL);
    if (!pInfo) {
        return ReturnForConvToPemcert2(pBioPem, Security_Cert_Invalid);
    }

    X509_INFO *itmp = nullptr;
    for (mp_int32 i = 0; i < sk_X509_INFO_num(pInfo); i++) {
        itmp = sk_X509_INFO_value(pInfo, i);
        if (itmp != nullptr && itmp->x509) {
            sk_X509_push(pCerts, itmp->x509);
            X509_up_ref(itmp->x509);
        }
    }
    sk_X509_INFO_pop_free(pInfo, X509_INFO_free);
    return ReturnForConvToPemcert2(pBioPem, SecuritySS_Success);
}

CURLcode SSLHandle::SslctxFunction(SSL_CTX *sslctx, const string& certText)
{
    // BIO is changed in PEM_read_bio_X509. So, need two BIO
    STACK_OF(X509) * pCerts = sk_X509_new_null();
    if (pCerts == nullptr) {
        return CURLE_FAILED_INIT;
    }

    SecuritySS_RC rc = ConvToX509(certText, pCerts);
    if (rc != SecuritySS_Success) {
        COMMLOG(OS_LOG_ERROR, "ConvToX509 fail, CURLE_SSL_CACERT_BADFILE.");
        return ReturnForSslCtxFunction(pCerts, CURLE_SSL_CACERT_BADFILE);
    }

    // Openssl will check is cert expired automatically, therefor needn't check here
    // Needn't to release pStore. Will core if release in this function
    X509_STORE *pStore = SSL_CTX_get_cert_store(sslctx);
    if (!pStore) {
        COMMLOG(OS_LOG_ERROR, "SSL_CTX_get_cert_store fail, CURLE_FAILED_INIT.");
        return ReturnForSslCtxFunction(pCerts, CURLE_FAILED_INIT);
    }

    X509 *pCert = nullptr;
    for (mp_int32 i = 0; i < sk_X509_num(pCerts); i++) {
        pCert = sk_X509_value(pCerts, i);
        if (X509_STORE_add_cert(pStore, pCert) == 0) {
            COMMLOG(OS_LOG_ERROR, "X509_STORE_add_cert fail, CURLE_FAILED_INIT.");
            return ReturnForSslCtxFunction(pCerts, CURLE_FAILED_INIT);
        }
    }

    return ReturnForSslCtxFunction(pCerts, CURLE_OK);
}

SecuritySS_RC SSLHandle::GetThumbprint(const string &certText, string &thumbprint)
{
    return GetGeneralThumbprint(certText, thumbprint, "SHA-1");
}

SecuritySS_RC SSLHandle::GetGeneralThumbprint(const mp_string& certText, mp_string& thumbprint,
    const mp_string& algorithm)
{
    thumbprint.clear();

    STACK_OF(X509) * pCerts = sk_X509_new_null();
    if (pCerts == nullptr) {
        return Security_Cert_Invalid;
    }

    SecuritySS_RC rc = ConvToX509(certText, pCerts);
    if (rc != SecuritySS_Success) {
        COMMLOG(OS_LOG_ERROR, "ConvToX509 fail, Security_Cert_Invalid.");
        return ReturnForFGetThumbprint(pCerts, Security_Cert_Invalid);
    }
    X509 *pCert;
    if (sk_X509_num(pCerts) < 1) {
        COMMLOG(OS_LOG_ERROR, "sk_X509_num less than 1, Security_Cert_Invalid.");
        return ReturnForFGetThumbprint(pCerts, Security_Cert_Invalid);
    }

    pCert = sk_X509_value(pCerts, 0);
    const EVP_MD *fprintType = nullptr;
    if (algorithm == "SHA-1") {
        fprintType = EVP_sha1();
    } else if (algorithm == "SHA-256") {
        fprintType = EVP_sha256();
    }
    if (fprintType == nullptr) {
        COMMLOG(OS_LOG_ERROR, "Thumbprint algorithm not apply.");
        return Security_Cert_Invalid;
    }
    unsigned char fprint[EVP_MAX_MD_SIZE] = {0};
    unsigned int fprintSize = 0;
    if (!X509_digest(pCert, fprintType, fprint, &fprintSize)) {
        COMMLOG(OS_LOG_ERROR, "X509_digest failed, Security_Cert_Invalid.");
        return ReturnForFGetThumbprint(pCerts, Security_Cert_Invalid);
    }
    char buffer[CERT_HANDLE_NUM_10] = {0};
    thumbprint.reserve(fprintSize * CERT_HANDLE_NUM_10);
    for (unsigned int i = 0; i < fprintSize; ++i) {
        sprintf_s(buffer, CERT_HANDLE_NUM_10, "%02x:", fprint[i]);
        thumbprint += string(buffer);
    }
    thumbprint.resize(thumbprint.length() - 1);
    return ReturnForFGetThumbprint(pCerts, SecuritySS_Success);
}

SecuritySS_RC SSLHandle::GetCerts(const string &certText, STACK_OF(X509) * pCerts, X509 *pCert)
{
    if (pCerts == nullptr) {
        return SecuritySS_InnerWrong;
    }

    SecuritySS_RC rc = ConvToX509(certText, pCerts);
    if (rc != SecuritySS_Success) {
        COMMLOG(OS_LOG_ERROR, "ConvToX509 fail, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, rc);
    }

    if (sk_X509_num(pCerts) < 1) {
        COMMLOG(OS_LOG_ERROR, "sk_X509_num less than 1, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    return SecuritySS_Success;
}

SecuritySS_RC SSLHandle::CheckWhetherCACert(const string &certText, bool &isCACert)
{
    isCACert = false;

    X509 *pCert = nullptr;
    STACK_OF(X509) * pCerts = sk_X509_new_null();
    if (GetCerts(certText, pCerts, pCert) != SecuritySS_Success) {
        COMMLOG(OS_LOG_ERROR, "GetCerts failed.");
        return SecuritySS_InnerWrong;
    }

    for (int i = 0; i < sk_X509_num(pCerts); i++) {
        pCert = sk_X509_value(pCerts, i);

        int crit = 0;
        BASIC_CONSTRAINTS *bcons = static_cast<BASIC_CONSTRAINTS *>(
            X509_get_ext_d2i(pCert, NID_basic_constraints, &crit, nullptr));
        if (bcons == nullptr) {
            COMMLOG(OS_LOG_ERROR, "Get basic constraints failed, Security_Cert_Invalid.");
            isCACert = false;
            return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
        }
        int ca = bcons->ca;
        COMMLOG(OS_LOG_DEBUG, "Get basic constraints success, bcons->ca=%d.", bcons->ca);
        BASIC_CONSTRAINTS_free(bcons);
        if (!ca) {  // not CA
            COMMLOG(OS_LOG_ERROR, "The cert is not CA.");
            isCACert = false;
            return ReturnForCheckCert(pCerts, SecuritySS_Success);
        }

        ASN1_BIT_STRING *keyUsageStr = static_cast<ASN1_BIT_STRING *>(
            X509_get_ext_d2i(pCert, NID_key_usage, nullptr, nullptr));
        if (keyUsageStr == nullptr) {
            COMMLOG(OS_LOG_DEBUG, "Get key usage, but the key usage does not exist.");
            isCACert = true;
            ASN1_BIT_STRING_free(keyUsageStr);
            continue;
        }

        unsigned short keyusage = 0;
        SetKeyUsage(*keyUsageStr, keyusage);
        ASN1_BIT_STRING_free(keyUsageStr);

        if (keyusage & KU_KEY_CERT_SIGN) {
            isCACert = true;
            continue;
        }

        isCACert = false;
        return ReturnForCheckCert(pCerts, SecuritySS_Success);
    }
    COMMLOG(OS_LOG_DEBUG, "isCACert=%d.", isCACert);
    return ReturnForCheckCert(pCerts, SecuritySS_Success);
}
/* END: Added by zhangyoupeng, 2016/06/12 */
/* BEGIN: Added by yulei, 2016/05/11   PN: add functions for CERT module */
/****************************************************************************
Function name          : SSLHandle::GetCertValidTime
Description            : Get cert's StartTime and ExpireTime
Input parameters       : certText: Cert's content
                         StartTime: notBefore
                         ExpireTime: notAfter
Output parameters      : StartTime: notBefore
                         ExpireTime: notAfter
Return value           : SecuritySS_RC
Updated Record         : None
****************************************************************************/
SecuritySS_RC SSLHandle::GetCertValidTime(const string &certText, time_t &StartTime, time_t &ExpireTime)
{
    if (NONE_CERT_STRING == certText) {
        COMMLOG(OS_LOG_WARN, "The value indicate delete the cert.");
        return SecuritySS_Success;
    }

    STACK_OF(X509) * pCerts = sk_X509_new_null();
    if (pCerts == nullptr) {
        return SecuritySS_InnerWrong;
    }

    SecuritySS_RC rc = ConvToX509(certText, pCerts);
    if (rc != SecuritySS_Success) {
        COMMLOG(OS_LOG_ERROR, "ConvToX509 fail, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, rc);
    }

    X509 *pCert;
    if (sk_X509_num(pCerts) < 1) {
        COMMLOG(OS_LOG_ERROR, "sk_X509_num less than 1, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    pCert = sk_X509_value(pCerts, 0);
    if (pCert == nullptr) {
        COMMLOG(OS_LOG_ERROR, "pCert is NULL.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    // Step 1. Get StartTime
    asn1_string_st *before = X509_get_notBefore(pCert);
    if (!before) {
        COMMLOG(OS_LOG_ERROR, "X509_get_notBefore fail, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    ASN1_UTCTIME *pBeforeTime = ASN1_STRING_dup(before);
    if (!pBeforeTime) {
        COMMLOG(OS_LOG_ERROR, "ASN1_STRING_dup for end time fail, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    // Convert time format
    StartTime = ASN1_TIME_ConvertToTimeT(*pBeforeTime);

    ASN1_UTCTIME_free(pBeforeTime);

    // Step 2. Call GetCertExpireTime to Get ExpireTime
    if (SecuritySS_Success != GetCertExpireTime(certText, ExpireTime)) {
        COMMLOG(OS_LOG_ERROR, "Get cert's expire time failed.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }
    return ReturnForCheckCert(pCerts, SecuritySS_Success);
}

/****************************************************************************
Function name          : SSLHandle::GetCertIssuer
Description            : Get cert's Issuer
Input parameters       : certText: Cert's content
                         issuer: issuer
Output parameters      : issuer: issuer
Return value           : SecuritySS_RC
Updated Record         : None
****************************************************************************/
SecuritySS_RC SSLHandle::GetCertIssuer(const string &certText, string &issuer)
{
    if (NONE_CERT_STRING == certText) {
        COMMLOG(OS_LOG_WARN, "The value indicate delete the cert.");
        return SecuritySS_Success;
    }

    STACK_OF(X509) * pCerts = sk_X509_new_null();
    if (pCerts == nullptr) {
        return SecuritySS_InnerWrong;
    }

    SecuritySS_RC rc = ConvToX509(certText, pCerts);
    if (rc != SecuritySS_Success) {
        COMMLOG(OS_LOG_ERROR, "ConvToX509 fail, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, rc);
    }

    X509 *pCert;
    if (sk_X509_num(pCerts) < 1) {
        COMMLOG(OS_LOG_ERROR, "sk_X509_num less than 1, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    pCert = sk_X509_value(pCerts, 0);
    char *pIssuer = X509_NAME_oneline(X509_get_issuer_name(pCert), 0, 0);
    if (!pIssuer) {
        COMMLOG(OS_LOG_ERROR, "Invalid Cert.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    issuer = string(pIssuer);

    OPENSSL_free(pIssuer);
    return ReturnForCheckCert(pCerts, SecuritySS_Success);
}

/****************************************************************************
Function name          : SSLHandle::GetCertSubject
Description            : Get cert's IssueTo (cert owner)
Input parameters       : certText: Cert's content
                         subject: IssueTo (cert owner)
Output parameters      : subject: IssueTo (cert owner)
Return value           : SecuritySS_RC
Updated Record         : None
****************************************************************************/
SecuritySS_RC SSLHandle::GetCertSubject(const string &certText, string &subject)
{
    if (NONE_CERT_STRING == certText) {
        COMMLOG(OS_LOG_WARN, "The value indicate delete the cert.");
        return SecuritySS_Success;
    }

    STACK_OF(X509) * pCerts = sk_X509_new_null();
    if (pCerts == nullptr) {
        return SecuritySS_InnerWrong;
    }

    SecuritySS_RC rc = ConvToX509(certText, pCerts);
    if (rc != SecuritySS_Success) {
        COMMLOG(OS_LOG_ERROR, "ConvToX509 fail, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, rc);
    }

    X509 *pCert;
    if (sk_X509_num(pCerts) < 1) {
        COMMLOG(OS_LOG_ERROR, "sk_X509_num less than 1, Security_Cert_Invalid.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    pCert = sk_X509_value(pCerts, 0);

    char *pSubject = X509_NAME_oneline(X509_get_subject_name(pCert), 0, 0);
    if (!pSubject) {
        COMMLOG(OS_LOG_ERROR, "Invalid Cert.");
        return ReturnForCheckCert(pCerts, Security_Cert_Invalid);
    }

    subject = string(pSubject);

    OPENSSL_free(pSubject);
    return ReturnForCheckCert(pCerts, SecuritySS_Success);
}

/****************************************************************************
Function name          : SSLHandle::VerifyCertContent
Description            : Verify Cert
Input parameters       : hostName: peer ip & port, exp: 1.1.1.1:443
                         certText: cert's content in text
Return value           : SecuritySS_RC
Updated Record         : None
****************************************************************************/
SecuritySS_RC SSLHandle::VerifyCertContent(const mp_string &hostName, const mp_string &certText, mp_string &fingerprint)
{
    COMMLOG(OS_LOG_DEBUG, "VerifyCertContent begin.certText is: %s", certText.c_str());

    SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
    if (!ctx) {
        COMMLOG(OS_LOG_ERROR, "Ctx is null.");
        return Security_Cert_Invalid;
    }

    SSL_CTX_set_verify_depth(ctx, CERT_HANDLE_NUM_10);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, SSLHandleVerifyCallback);
    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

    if (SecuritySS_Success != SSLHandle::AddCertToCtx(ctx, certText)) {
        COMMLOG(OS_LOG_ERROR, "Add cert to ctx error.");
        SslCtxFree(ctx);
        return Security_Cert_Invalid;
    }

    BIO *sbio = BIO_new_ssl_connect(ctx);
    SSL *ssl = nullptr;

    BIO_get_ssl(sbio, &ssl);
    if (!ssl) {
        COMMLOG(OS_LOG_ERROR, "Can't get ssl form bio.");
        SslCtxFree(ctx);
        BioFreeAll(sbio);
        return Security_Cert_Invalid;
    }
    COMMLOG(OS_LOG_DEBUG, "Attempt to connect %s.", hostName.c_str());
    BIO_set_conn_hostname(sbio, hostName.c_str());

    if (BIO_do_handshake(sbio) <= 0) {
        COMMLOG(OS_LOG_ERROR, "BIO_do_connect failed.");
        SslCtxFree(ctx);
        BioFreeAll(sbio);
        return Security_Cert_Invalid;
    }

    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        COMMLOG(OS_LOG_ERROR, "SSL_get_verify_result failed.");
        SslCtxFree(ctx);
        BioFreeAll(sbio);
        return Security_Cert_Invalid;
    }

    COMMLOG(OS_LOG_DEBUG, "SSL_get_verify_result success.");

    GetThumbprint(certText, fingerprint);  // Get the fingerprint of the match cert.

    COMMLOG(OS_LOG_DEBUG, "VerifyCertContent end.");
    SslCtxFree(ctx);
    BioFreeAll(sbio);
    SecuritySS_RC ret = SecuritySS_Success;
    return ret;
}

time_t SSLHandle::ASN1_UTCTIME_ConvertToTimeT(const ASN1_UTCTIME &tmASN1)
{
    const char *v;
    int i;
    time_t tmRet;

    i = tmASN1.length;
    v = (const char *)tmASN1.data;
    if (i < CERT_HANDLE_NUM_10) {
        return 0;
    }

    for (i = 0; i < CERT_HANDLE_NUM_10; i++) {
        if ((v[i] > '9') || (v[i] < '0')) {
            return 0;
        }
    }

    int y = (v[0] - '0') * CERT_HANDLE_NUM_10 + (v[1] - '0');
    if (y < CERT_HANDLE_NUM_50) {
        y += CERT_HANDLE_NUM_100;
    }
    int M = (v[CERT_HANDLE_NUM_2] - '0') * CERT_HANDLE_NUM_10 + (v[CERT_HANDLE_NUM_3] - '0');
    if ((M > CERT_HANDLE_NUM_12) || (M < 1)) {
        return 0;
    }
    int d = (v[CERT_HANDLE_NUM_4] - '0') * CERT_HANDLE_NUM_10 + (v[CERT_HANDLE_NUM_5] - '0');
    int h = (v[CERT_HANDLE_NUM_6] - '0') * CERT_HANDLE_NUM_10 + (v[CERT_HANDLE_NUM_7] - '0');
    int m = (v[CERT_HANDLE_NUM_8] - '0') * CERT_HANDLE_NUM_10 + (v[CERT_HANDLE_NUM_9] - '0');
    int s = ASN1_UTCTIME_ConvertToTimeT_Inner(tmASN1, v);

    struct tm t;
    t.tm_year = y;
    t.tm_mon = M - 1;
    t.tm_mday = d;
    t.tm_hour = h;
    t.tm_min = m;
    t.tm_sec = s;

    tmRet = (mktime(&t));
    return tmRet;
}

int SSLHandle::ASN1_UTCTIME_ConvertToTimeT_Inner(const ASN1_UTCTIME &tmASN1, const char v[])
{
    int s = 0;
    if (tmASN1.length >= CERT_HANDLE_NUM_12 && (v[CERT_HANDLE_NUM_10] >= '0') && (v[CERT_HANDLE_NUM_10] <= '9') &&
        (v[CERT_HANDLE_NUM_11] >= '0') && (v[CERT_HANDLE_NUM_11] <= '9')) {
        s = (v[CERT_HANDLE_NUM_10] - '0') * CERT_HANDLE_NUM_10 + (v[CERT_HANDLE_NUM_11] - '0');
    }

    return s;
}

time_t SSLHandle::ASN1_GENERALIZEDTIME_ConvertToTimeT(const ASN1_GENERALIZEDTIME &tmASN1)
{
    char *v;
    int i;
    time_t tmRet;

    i = tmASN1.length;
    v = reinterpret_cast<char *>(tmASN1.data);

    if (i < CERT_HANDLE_NUM_12) {
        return 0;
    }
    for (i = 0; i < CERT_HANDLE_NUM_12; i++) {
        if ((v[i] > '9') || (v[i] < '0')) {
            return 0;
        }
    }

    int M = (v[CERT_HANDLE_NUM_4] - '0') * CERT_HANDLE_NUM_10 + (v[CERT_HANDLE_NUM_5] - '0');
    if ((M > CERT_HANDLE_NUM_12) || (M < 1)) {
        return 0;
    }
    int d = (v[CERT_HANDLE_NUM_6] - '0') * CERT_HANDLE_NUM_10 + (v[CERT_HANDLE_NUM_7] - '0');
    int h = (v[CERT_HANDLE_NUM_8] - '0') * CERT_HANDLE_NUM_10 + (v[CERT_HANDLE_NUM_9] - '0');
    int m = (v[CERT_HANDLE_NUM_10] - '0') * CERT_HANDLE_NUM_10 + (v[CERT_HANDLE_NUM_11] - '0');
    int s = ASN1_GENERALIZEDTIME_ConvertToTimeT_Inner(tmASN1, v);

    int y = (v[0] - '0') * CERT_HANDLE_NUM_1000 + (v[1] - '0') * CERT_HANDLE_NUM_100 +
            (v[CERT_HANDLE_NUM_2] - '0') * CERT_HANDLE_NUM_10 + (v[CERT_HANDLE_NUM_3] - '0');
    struct tm t;
    t.tm_year = y - CERT_HANDLE_NUM_1900;
    t.tm_mon = M - 1;
    t.tm_mday = d;
    t.tm_hour = h;
    t.tm_min = m;
    t.tm_sec = s;

    tmRet = (mktime(&t));

    return tmRet;
}

mp_void SSLHandle::ConvertToTimeT_Inner(const ASN1_GENERALIZEDTIME &tmASN1, char v[])
{
    /* Check for fractions of seconds. */
    if (tmASN1.length >= CERT_HANDLE_NUM_15 && v[CERT_HANDLE_NUM_14] == '.') {
        int l = tmASN1.length;
        char *fTime = &v[CERT_HANDLE_NUM_14]; /* The decimal point. */
        int f_len = 1;
        while (CERT_HANDLE_NUM_14 + f_len < l && fTime[f_len] >= '0' && fTime[f_len] <= '9') {
            ++f_len;
        }
    }
}

int SSLHandle::ASN1_GENERALIZEDTIME_ConvertToTimeT_Inner(const ASN1_GENERALIZEDTIME &tmASN1, char v[])
{
    int s = 0;

    if (tmASN1.length >= CERT_HANDLE_NUM_14 && (v[CERT_HANDLE_NUM_12] >= '0') && (v[CERT_HANDLE_NUM_12] <= '9') &&
        (v[CERT_HANDLE_NUM_13] >= '0') && (v[CERT_HANDLE_NUM_13] <= '9')) {
        s = (v[CERT_HANDLE_NUM_12] - '0') * CERT_HANDLE_NUM_10 + (v[CERT_HANDLE_NUM_13] - '0');
        ConvertToTimeT_Inner(tmASN1, v);
    }

    return s;
}

time_t SSLHandle::ASN1_TIME_ConvertToTimeT(const ASN1_TIME &tmASN1)
{
    if (tmASN1.type == V_ASN1_UTCTIME) {
        return ASN1_UTCTIME_ConvertToTimeT(tmASN1);
    }
    if (tmASN1.type == V_ASN1_GENERALIZEDTIME) {
        return ASN1_GENERALIZEDTIME_ConvertToTimeT(tmASN1);
    }

    return 0;
}
/* END: Added by yulei, 2015/05/11 */
SecuritySS_RC SSLHandle::AddCertToCtx(SSL_CTX *ctx, const mp_string &certText)
{
    BIO *pBioPem = BIO_new_mem_buf(
        reinterpret_cast<void *>(const_cast<char *>(certText.c_str())), static_cast<int>(certText.length()));
    if (!pBioPem) {
        COMMLOG(OS_LOG_ERROR, "BIO_new_mem_buf fail for pem, Security_Cert_Invalid.");
        return Security_Cert_Invalid;
    }

    STACK_OF(X509_INFO) * pInfo = PEM_X509_INFO_read_bio(pBioPem, nullptr, nullptr, nullptr);
    if (!pInfo) {
        COMMLOG(OS_LOG_ERROR, "Cert info get failed.");
        BioFree(pBioPem);
        return Security_Cert_Invalid;
    }

    X509_STORE *x509_store = SSL_CTX_get_cert_store(ctx);
    X509_INFO *itmp = nullptr;
    for (int i = 0; i < sk_X509_INFO_num(pInfo); i++) {
        itmp = sk_X509_INFO_value(pInfo, i);
        if (itmp != nullptr && itmp->x509) {
            COMMLOG(OS_LOG_DEBUG, "Add cert %d to store.", i);
            if (X509_STORE_add_cert(x509_store, itmp->x509) == 0) {
                COMMLOG(OS_LOG_ERROR, "ADD cert %d to store failed!.", i);
                SkX509InfoPopFree(pInfo);
                BioFree(pBioPem);
                return SecuritySS_InnerWrong;
            }
        }
    }
    SkX509InfoPopFree(pInfo);
    BioFree(pBioPem);
    return SecuritySS_Success;
}

#endif
