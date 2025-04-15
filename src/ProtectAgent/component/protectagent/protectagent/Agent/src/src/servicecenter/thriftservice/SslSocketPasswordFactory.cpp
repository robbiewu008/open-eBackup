/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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

