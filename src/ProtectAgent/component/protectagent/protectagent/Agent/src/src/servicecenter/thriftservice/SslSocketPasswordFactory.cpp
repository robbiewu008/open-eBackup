/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file SslSocketPasswordFactory.cpp
 * @brief  Implement for TSSLSocketFactory
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#include <thriftservice/detail/SslSocketPasswordFactory.h>
#include "common/Log.h"

using namespace certificateservice;
namespace thriftservice {
namespace detail {
SslSocketPasswordFactory::SslSocketPasswordFactory(SSLProtocol protocol)
    : TSSLSocketFactory(protocol)
{
}

void SslSocketPasswordFactory::getPassword(std::string& password, int size)
{
    m_handler->GetCertificateConfig(CertificateConfig::PASSWORD, password);
}

bool SslSocketPasswordFactory::LoadServerCertificate()
{
    bool ret = false;
    try {
        std::string suite;
        m_handler->GetCertificateConfig(CertificateConfig::ALGORITEHM_SUITE, suite);
        std::string use = m_handler->GetCertificateFile(CertificateType::USE_CRETIFICATE_FILE);
        std::string key = m_handler->GetCertificateFile(CertificateType::KEY_FILE);
        loadCertificate(use.c_str());
        overrideDefaultPasswordCallback();
        loadPrivateKey(key.c_str());
        ciphers(suite);
        server(true);
        ret = true;
    } catch (TSSLException& ex) {
        ERRLOG("Start thrift client failed, TSSLException Exception. %s", ex.what());
    } catch (const std::exception& ex) {
        ERRLOG("Start thrift client failed, Standard C++ Exception. %s", ex.what());
    } catch (...) {
        ERRLOG("Start thrift client failed. Unknown exception.");
    }
    return ret;
}

bool SslSocketPasswordFactory::LoadClientCertificate()
{
    bool ret = false;
    try {
        std::string suite;
        std::string use = m_handler->GetCertificateFile(CertificateType::USE_CRETIFICATE_FILE);
        std::string key = m_handler->GetCertificateFile(CertificateType::KEY_FILE);
        std::string root = m_handler->GetCertificateFile(CertificateType::TRUSTE_CRETIFICATE_FILE);
        m_handler->GetCertificateConfig(CertificateConfig::ALGORITEHM_SUITE, suite);
        overrideDefaultPasswordCallback();
        loadTrustedCertificates(root.c_str());
        loadCertificate(use.c_str());
        loadPrivateKey(key.c_str());
        ciphers(suite);
        authenticate(true);
        ret = true;
    } catch (TSSLException& ex) {
        ERRLOG("Start thrift client failed, TSSLException Exception. %s", ex.what());
    } catch (const std::exception& ex) {
        ERRLOG("Start thrift client failed, Standard C++ Exception. %s", ex.what());
    } catch (...) {
        ERRLOG("Start thrift client failed. Unknown exception.");
    }
    return ret;
}
}
}

