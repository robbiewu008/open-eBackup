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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"

#include "repository_handlers/filesystem/FileSystemHandler.h"
#include "common/cert_mgr/CertMgr.h"

#include "repository_handlers/mock/FileSystemHandlerMock.h"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;
using namespace VirtPlugin;

namespace HDT_TEST {
class CertMgrTest : public testing::Test {
protected:
    void SetUp() {}
    void TearDown() {}
};

/*
 * 测试用例： 证书落盘成功
 * 前置条件： 证书内容格式正确
 * CHECK点： SaveCertInfo返回值为SUCCESS
 */
const std::string temp_path = "/opt/DataBackup/ProtectClient/Plugins/VirtualizationPlugin//tmp/cert";

TEST_F(CertMgrTest, ParseCertInfoSuccess)
{
    std::string certInfo = "{\"enableCert\":\"1\",\"certification\":\"0123456789\",\"storages\":\"\"}";
    std::shared_ptr<CertManger> certMgr = std::make_shared<CertManger>();
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(Return(true));
    CertInfo cert;
    certMgr->m_fHandler = fsHandler;
    int ret = certMgr->ParseCertInfo(certInfo, cert);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(certMgr->GetSpecifyCerPath("1"), temp_path+"/1.pem");
    EXPECT_EQ(certMgr->GetSpecifyRclPath("1"), temp_path+"/1.crl");
    EXPECT_EQ(certMgr->GetCPSCertPath(), "/opt/DataBackup/ProtectClient/Plugins/VirtualizationPlugin//cert/cinder/cinder.pem");
}

/*
 * 测试用例： 证书落盘失败
 * 前置条件： 证书内容格式错误
 * CHECK点： SaveCertInfo返回值为FAILED
 */
TEST_F(CertMgrTest, ParseCertInfoFailed)
{
    std::string certInfo = "123456789";
    std::shared_ptr<CertManger> certMgr = std::make_shared<CertManger>();
    CertInfo cert;
    int ret = certMgr->ParseCertInfo(certInfo, cert);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(certMgr->GetCPSCertPath(), "");
}

static std::string g_cert = ""
    "-----BEGIN CERTIFICATE-----"
    "MIIDrTCCApWgAwIBAgIIF/WjXPtP1AkwDQYJKoZIhvcNAQELBQAwZDELMAkGA1UE"
    "kJpwfyU2ASAoJc4EtLpcW+EsqAa/FlUWuKNrjgtqBuYQ2khlZA/Hw974g3msm16s"
    "dmSS6AWXsGw89JiUkCrbCN7AXzJHmQnZpcXPZ0DJV1G2"
    "-----END CERTIFICATE-----";

static std::string g_crl = ""
    "-----BEGIN X509 CRL-----"
    "MIIB9jCB3wIBATANBgkqhkiG9w0BAQsFADBkMQswCQYDVQQGEwJDTjELMAkGA1UE"
    "CBMCQ0QxCzAJBgNVBAcTAkNEMQswCQYDVQQKEwJDRDEMMAoGA1UECxMDMTIzMQww"
    "JBG6yX4hsKVLzJDH8m6HIF8raA/AqnljW0c="
    "-----END X509 CRL-----";

/*
 * 测试用例： 证书信息保存到文件
 * 前置条件： 读写文件正常
 * CHECK点： 自创建目录情况和已创建目录情况均成功
 */
TEST_F(CertMgrTest, SaveCertToFileInfoSuccess)
{
    std::string name = "ca";
    std::shared_ptr<CertManger> certMgr = std::make_shared<CertManger>();

    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Write(_)).Times(4).WillOnce(Return(g_cert.size()))
                                              .WillOnce(Return(g_crl.size()))
                                              .WillOnce(Return(g_cert.size()))
                                              .WillOnce(Return(g_crl.size()));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).Times(2).WillOnce(Return(false))     // 未创建目录
                                                    .WillOnce(Return(true));     // 已创建目录
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    certMgr->m_fHandler = fsHandler;

    certMgr->m_certification = g_cert;
    certMgr->m_revocationList = g_crl;

    // 自创建目录
    int ret = certMgr->SaveCertToFile(name);
    EXPECT_EQ(ret, true);

    // 已创建目录
    ret = certMgr->SaveCertToFile(name);
    EXPECT_EQ(ret, true);

    certMgr->ClearResource();
}

/*
 * 测试用例： 证书信息保存到文件失败
 * 前置条件： 证书内容格式正确
 * CHECK点： SaveCertInfo返回值为SUCCESS
 */
TEST_F(CertMgrTest, SaveCertToFileFail)
{
    std::string name = "ca";
    std::shared_ptr<CertManger> certMgr = std::make_shared<CertManger>();

    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).Times(2).WillOnce(Return(SUCCESS))         // 文件打开成功但写文件失败
                                               .WillOnce(Return(FAILED));         // 文件打开失败
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Write(_)).Times(1).WillOnce(Return(0));                     // 写文件失败
    EXPECT_CALL(*fsHandler, IsDirectory(_)).Times(2).WillOnce(Return(false))     // 未创建目录
                                                    .WillOnce(Return(true));     // 已创建目录
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    certMgr->m_fHandler = fsHandler;

    certMgr->m_certification = g_cert;

    // 打开文件失败
    int ret = certMgr->SaveCertToFile(name);
    EXPECT_EQ(ret, false);

    // 写文件失败
    ret = certMgr->SaveCertToFile(name);
    EXPECT_EQ(ret, false);
}

}