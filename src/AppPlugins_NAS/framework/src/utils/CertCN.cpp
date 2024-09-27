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
#include "CertCN.h"
#include "log/Log.h"

const std::string CRT_USER_NAME = "/CN=";
void InnerFreeGetHostMem(BIO *key, X509 *pCert, char *pSubject)
{
    if (key != nullptr) {
        BIO_free_all(key);
    }
    if (pCert != nullptr) {
        X509_free(pCert);
    }
    if (pSubject != nullptr) {
        OPENSSL_free(pSubject);
    }
}


void GetHostFromCert(std::string &certPath, std::string &hostName)
{
    BIO *key = nullptr;
    X509 *pCert = nullptr;
    std::string errorStr = "";
    char *pSubject = nullptr;

    do {
        key = BIO_new_file(certPath.c_str(), "r");
        if (key == nullptr) {
            errorStr = "BIO_new_file key is nullptr";
            break;
        }

        pCert = PEM_read_bio_X509(key, nullptr, nullptr, nullptr);
        if (pCert == nullptr) {
            errorStr = "PEM_read_bio_X509 failed.";
            break;
        }

        X509_NAME *name = X509_get_subject_name(pCert);
        if (name == nullptr) {
            errorStr = "get x509 subject name failed.";
            break;
        }
        pSubject = X509_NAME_oneline(name, nullptr, 0);
        if (pSubject == nullptr) {
            errorStr = "Invalid Cert.";
            break;
        }

        INFOLOG("subjectName:%s", pSubject);

        std::string tmpHost(pSubject);
        int index = tmpHost.find(CRT_USER_NAME);
        if (index == std::string::npos) {
            errorStr = "Cannot find hostName flag.";
            break;
        }

        index += CRT_USER_NAME.length();
        hostName = tmpHost.substr(index, tmpHost.length() - index);
        if (hostName.empty()) {
            errorStr = "host name is empty.";
            break;
        }

        index = hostName.find("/");
        if (index == std::string::npos) {
            break;
        }
        hostName = hostName.substr(0, index);
    } while (0);

    InnerFreeGetHostMem(key, pCert, pSubject);
    if (!errorStr.empty()) {
        ERRLOG("%s", errorStr.c_str());
    }
}