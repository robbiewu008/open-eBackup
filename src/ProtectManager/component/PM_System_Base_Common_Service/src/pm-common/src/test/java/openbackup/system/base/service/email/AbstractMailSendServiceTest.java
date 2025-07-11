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
package openbackup.system.base.service.email;

import openbackup.system.base.service.ApplicationContextService;
import openbackup.system.base.service.email.AbstractMailSendService;
import openbackup.system.base.service.email.entity.RemoteNotifyServer;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * 功能描述
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {ApplicationContextService.class})
@Ignore
public class AbstractMailSendServiceTest {
    @Test
    @Ignore
    public void send_mail_success() {
        RemoteNotifyServer remoteNotifyServer = new RemoteNotifyServer();
        remoteNotifyServer.setId(1L);
        remoteNotifyServer.setServer("8.40.145.169");
        remoteNotifyServer.setPort(25);
        remoteNotifyServer.setEmailFrom("ldap-send@8.40.145.169");
        remoteNotifyServer.setUserName("ldap-send@8.40.145.169");
        remoteNotifyServer.setPassword("123456");
        remoteNotifyServer.setIsSslEnable(false);
        remoteNotifyServer.setIsTlsEnable(false);
        remoteNotifyServer.setProxyEnable(false);
        remoteNotifyServer.setValidateEnable(true);
        int errorCode =
            AbstractMailSendService.sendEmail(remoteNotifyServer, "测试邮件", "test Email content", "test@8.40.145.169");
        Assert.assertEquals(errorCode, 0);
    }

    @Test
    public void send_proxy_mail_fail() {
        RemoteNotifyServer remoteNotifyServer = new RemoteNotifyServer();
        remoteNotifyServer.setId(1L);
        remoteNotifyServer.setServer("8.40.145.169");
        remoteNotifyServer.setPort(25);
        remoteNotifyServer.setEmailFrom("ldap-send@8.40.145.169");
        remoteNotifyServer.setUserName("ldap-send@8.40.145.169");
        remoteNotifyServer.setPassword("123456");
        remoteNotifyServer.setIsSslEnable(false);
        remoteNotifyServer.setIsTlsEnable(false);
        remoteNotifyServer.setProxyEnable(true);
        remoteNotifyServer.setProxyServer("1.1.1.1");
        remoteNotifyServer.setProxyPort("20");
        remoteNotifyServer.setValidateEnable(true);
        int errorCode =
            AbstractMailSendService.sendEmail(remoteNotifyServer, "测试邮件", "test Email content", "test@8.40.145.169");
        Assert.assertEquals(errorCode, 8);
    }

    @Test
    @Ignore
    public void send_mail_ssl_fail() {
        RemoteNotifyServer remoteNotifyServer = new RemoteNotifyServer();
        remoteNotifyServer.setId(1L);
        remoteNotifyServer.setServer("8.40.99.119");
        remoteNotifyServer.setPort(25);
        remoteNotifyServer.setEmailFrom("smtp-send@huawei.com");
        remoteNotifyServer.setUserName("smtp-send@huawei.com");
        remoteNotifyServer.setPassword("123456");
        remoteNotifyServer.setIsSslEnable(true);
        remoteNotifyServer.setIsTlsEnable(false);
        remoteNotifyServer.setProxyEnable(false);
        remoteNotifyServer.setValidateEnable(true);
        int errorCode =
            AbstractMailSendService.sendEmail(remoteNotifyServer, "测试邮件", "test Email content", "smtp-test@huawei.com");
        Assert.assertEquals(errorCode, 15);
    }

    @Test
    @Ignore
    public void send_mail_with_attachment_success() {
        RemoteNotifyServer remoteNotifyServer = new RemoteNotifyServer();
        remoteNotifyServer.setId(1L);
        remoteNotifyServer.setServer("8.40.145.169");
        remoteNotifyServer.setPort(25);
        remoteNotifyServer.setEmailFrom("ldap-send@8.40.145.169");
        remoteNotifyServer.setUserName("ldap-send@8.40.145.169");
        remoteNotifyServer.setPassword("123456");
        remoteNotifyServer.setIsSslEnable(false);
        remoteNotifyServer.setIsTlsEnable(false);
        remoteNotifyServer.setProxyEnable(false);
        remoteNotifyServer.setValidateEnable(true);
        int errorCode = AbstractMailSendService.sendAttachmentEmail(remoteNotifyServer, "test attachment title",
            "带附件的邮件正文", "test@8.40.145.169", new String[] {"D:\\test.txt"});
        Assert.assertEquals(errorCode, 0);
    }
}
