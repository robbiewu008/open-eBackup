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

/**
 * 邮件常量
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-22
 */
public class EmailConstants {
    /**
     * 邮箱服务器地址
     */
    public static final String MAIL_HOST = "mail.smtp.host";

    /**
     * 邮箱服务器端口
     */
    public static final String MAIL_PORT = "mail.smtp.port";

    /**
     * 是否校验CN
     */
    public static final String CHECK_HOST_NAME = "mail.smtp.ssl.checkserveridentity";

    /**
     * 需要证书
     */
    public static final String STARTTLS_REQUIRED = "mail.smtp.starttls.required";

    /**
     * 邮件发件人
     */
    public static final String MAIL_SMTP_FROM = "mail.smtp.from";

    /**
     * 邮箱鉴权信息
     */
    public static final String MAIL_SMTP_AUTH = "mail.smtp.auth";

    /**
     * 邮箱用户名
     */
    public static final String MAIL_SMTP_USER = "mail.smtp.user";

    /**
     * 邮箱用户名密码
     */
    public static final String MAIL_SMTP_PASSWORD = "mail.smtp.password";

    /**
     * 邮箱用户密码
     */
    public static final String MAIL_TRANSPORT_STARTTLS_ENABLE = "mail.smtp.starttls.enable";

    /**
     * 是否启用ssl
     */
    public static final String MAIL_SMTP_SSL_ENABLE = "mail.smtp.ssl.enable";

    /**
     * socketFactory
     */
    public static final String MAIL_SMTP_SSL_SOCKET_FACTORY = "mail.smtp.ssl.socketFactory";

    /**
     * socketFactory的回调
     */
    public static final String MAIL_SMTP_SSL_SOCKET_FALLBACK = "mail.smtp.ssl.socketFactory.fallback";

    /**
     * socketFactory的class
     */
    public static final String MAIL_SMTP_SSL_SOCKET_FACTORY_CLASS = "mail.smtp.ssl.socketFactory.class";

    /**
     * socketFactory
     */
    public static final String MAIL_SMTP_SSL_SOCKET_FACTORY_PORT = "mail.smtp.ssl.socketFactory.port";

    /**
     * timeout
     */
    public static final String MAIL_SNMTP_TIME_OUT = "mail.smtp.timeout";

    /**
     * connectiontimeout
     */
    public static final String MAIL_SMTP_CONNECT_TIME_OUT = "mail.smtp.connectiontimeout";

    /**
     * writetimeout
     */
    public static final String MAIL_SMTP_WRITE_TIME_OUT = "mail.smtp.writetimeout";

    /**
     * 传输协议
     */
    public static final String MAIL_TRANSPORT_PROTOCOL = "mail.transport.protocol";

    /**
     * 协议
     */
    public static final String SMTP_PROTOCOL = "smtp";

    /**
     * sendpartial
     */
    public static final String MAIL_SMTP_SENDPARTIAL = "mail.smtp.sendpartial";

    /**
     * SMTPS_sendpartial
     */
    public static final String MAIL_SMTPS_SENDPARTIAL = "mail.smtps.sendpartial";

    private EmailConstants() {
    }
}
