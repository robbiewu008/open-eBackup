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
#ifndef S3_IO_PARAMS_H
#define S3_IO_PARAMS_H
#include <string>
#include <boost/scoped_array.hpp>
// #include "protocol/BasicTypes.pb.h"
#include "IOCommonDef.h"
#include "eSDKOBS.h"

namespace Module {
struct S3IOParams {
public:
    S3IOParams()
    {}

    virtual ~S3IOParams();
    std::string host;
    std::string bucket;
    std::string userName;
    std::string passWord;
    obs_protocol protocol;
    std::string cert;
    obs_uri_style uriStyle;
    std::string bucketFullName;
    uint64_t uploadRateLimit {0};
    uint64_t downloadRateLimit {0};
    HTTP_PROXY_INFO HttpProxyInfo;
    SPEED_UP_INFO SpeedUpInfo;
};

typedef struct S3BucketContextProxy : obs_options {
    S3BucketContextProxy();
    S3BucketContextProxy(const S3IOParams &params);
    virtual ~S3BucketContextProxy();
    bool Init(const std::string &host, const std::string &bucket, const std::string &accessKey,
        const std::string &secureKey, const std::string &certificatePath, obs_protocol prot,
        const obs_uri_style &style);

    void SetBucketPath(const std::string &path)
    {
        m_Bucket = path;
        bucket_options.bucket_name = const_cast<char *>(m_Bucket.c_str());
    }
    void PrintBucketContext() const;
    int GetBbrSwitch() const;
    std::string GetProxyAddress() const;
    std::string GetProxyUser() const;

protected:
    std::string GetPlainPasswd(const std::string &secureKey);
    char *ReadCertificateInfo(const std::string &CertificatePath);
    void GetS3TimeOut(int &TimeOut, int &ConnectTimeOut);

public:
    std::string m_Host;
    std::string m_Bucket;
    std::string m_UserName;
    std::string m_PassWord;
    std::string m_FullHost;
    std::string m_FullAuth;
    obs_protocol m_Protocol;
    HTTP_PROXY_INFO m_Proxy;
    SPEED_UP_INFO m_Speed;
    boost::scoped_array<char> m_SpCertificate;
} S3BucketContextProxy;
} // namespace Module
#endif