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
#include "SslSocketPasswordFactory.h"
#include "log/Log.h"

using namespace std;
using namespace certificateservice;
using namespace apache::thrift::transport;

namespace {
    constexpr auto MODULE = "SSLSocketPasswordFactory";
}

namespace thriftservice {
namespace detail {
SslSocketPasswordFactory::SslSocketPasswordFactory(SSLProtocol protocol) : TSSLSocketFactory(protocol)
{}

void SslSocketPasswordFactory::getPassword(std::string& password, int size)
{
    size;
    m_handler->GetCertificateConfig(CertificateConfig::PASSWORD, password);
}

bool SslSocketPasswordFactory::LoadServerCertificate()
{
    string suite;
    bool ret = false;
    HCP_Log(INFO, MODULE) << "Loading server certificate suite!" << HCPENDLOG;
    try {
        string suite;
        m_handler->GetCertificateConfig(CertificateConfig::ALGORITHM_SUITE, suite);
        string certificate = m_handler->GetCertificateFile(CertificateType::USE_CERTIFICATE_FILE);
        loadCertificate(certificate.c_str());
        overrideDefaultPasswordCallback();
        string privateKey = m_handler->GetCertificateFile(CertificateType::KEY_FILE);
        loadPrivateKey(privateKey.c_str());
        ciphers(suite);
        server(true);
        ret = true;
    } catch (TSSLException& ex) {
        HCP_Log(ERR, MODULE) << "LoadServerCertificate failed, TSSLException Exception : "
            << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
    } catch (const std::exception& ex) {
        HCP_Log(ERR, MODULE) << "LoadServerCertificate failed, Standard C++ Exception. "
            << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
    } catch (...) {
        HCP_Log(ERR, MODULE) << "LoadServerCertificate failed. Unknown exception. " << HCPENDLOG;
    }
    return ret;
}

bool SslSocketPasswordFactory::LoadClientCertificate()
{
    bool ret = false;
    try {
        string suite;
        m_handler->GetCertificateConfig(CertificateConfig::ALGORITHM_SUITE, suite);
        string certificate = m_handler->GetCertificateFile(CertificateType::USE_CERTIFICATE_FILE);
        loadCertificate(certificate.c_str());
        overrideDefaultPasswordCallback();
        string privateKey = m_handler->GetCertificateFile(CertificateType::KEY_FILE);
        loadPrivateKey(privateKey.c_str());
        string trustedCertificates =  m_handler->GetCertificateFile(CertificateType::TRUSTE_CERTIFICATE_FILE);
        loadTrustedCertificates(trustedCertificates.c_str());
        ciphers(suite);
        authenticate(true);
        ret = true;
    } catch (TSSLException& ex) {
        HCP_Log(ERR, MODULE) << "LoadClientCertificate failed, TSSLException Exception. "
            << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
    } catch (const std::exception& ex) {
        HCP_Log(ERR, MODULE) << "LoadClientCertificate failed, Standard C++ Exception. "
            << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
    } catch (...) {
        HCP_Log(ERR, MODULE) << "LoadClientCertificate failed. Unknown exception. " << HCPENDLOG;
    }
    return ret;
}
}
}
