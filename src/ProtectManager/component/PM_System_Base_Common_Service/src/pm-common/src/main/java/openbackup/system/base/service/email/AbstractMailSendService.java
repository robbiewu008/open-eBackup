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

import jakarta.mail.Address;
import jakarta.mail.AuthenticationFailedException;
import jakarta.mail.Authenticator;
import jakarta.mail.Message;
import jakarta.mail.MessagingException;
import jakarta.mail.PasswordAuthentication;
import jakarta.mail.Session;
import jakarta.mail.Transport;
import jakarta.mail.internet.InternetAddress;
import jakarta.mail.internet.MimeBodyPart;
import jakarta.mail.internet.MimeMessage;
import jakarta.mail.internet.MimeMultipart;
import jakarta.mail.internet.MimeUtility;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.SecurityUtil;
import openbackup.system.base.service.email.entity.RemoteNotifyServer;

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.exception.ExceptionUtils;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.util.List;
import java.util.Properties;

import javax.activation.CommandMap;
import javax.activation.MailcapCommandMap;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLHandshakeException;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

/**
 * 邮件发送服务
 *
 */
@Slf4j
public abstract class AbstractMailSendService {
    private static final int VALUE_2 = 2;

    private static final String TIME_OUT = "5000";

    private static final String EMAIL_CHARSET = "GB2312";

    private static final String TRUE = "true";

    private static final String FALSE = "false";

    private static final String PROXY_SET = "proxySet";

    private static final String SOCKS_PROXY_HOST = "socksProxyHost";

    private static final String SOCKS_PROXY_PORT = "socksProxyPort";

    /**
     * 发送邮件
     *
     * @param remoteNotifyServer 邮件服务器设置模型
     * @param title title
     * @param content content
     * @param receiver receiver
     * @return int
     */
    public static int sendEmail(RemoteNotifyServer remoteNotifyServer, String title, String content, String receiver) {
        try {
            int errorCode = sentSingleEmail(remoteNotifyServer, title, content, receiver);
            log.debug("send: completed!");
            return errorCode;
        } finally {
            Properties properties = System.getProperties();
            if (remoteNotifyServer.isProxyEnable()) {
                properties.remove(PROXY_SET);
                properties.remove(SOCKS_PROXY_HOST);
                properties.remove(SOCKS_PROXY_PORT);
            }
        }
    }

    /**
     * 发送邮件
     *
     * @param remoteNotifyServer 邮件服务器设置模型
     * @param title 邮件主题
     * @param content 邮件内容
     * @param receiver 收件人地址
     * @param attachments 附件
     * @return int
     */
    public static int sendAttachmentEmail(RemoteNotifyServer remoteNotifyServer, String title, String content,
            String receiver, String[] attachments) {
        try {
            int errorCode = EmailCodes.ERROR_CODE_EXCEPTION;
            int count = 0;
            while (errorCode != 0 && count < VALUE_2) {
                errorCode = sendSingleAttachmentEmail(remoteNotifyServer, title, content, receiver, attachments);
                count++;
            }
            log.debug("send: completed!");
            return errorCode;
        } finally {
            Properties properties = System.getProperties();
            if (remoteNotifyServer != null && remoteNotifyServer.isProxyEnable()) {
                properties.remove(PROXY_SET);
                properties.remove(SOCKS_PROXY_HOST);
                properties.remove(SOCKS_PROXY_PORT);
            }
        }
    }

    /**
     * 发送邮件
     *
     * @param remoteNotifyServer 邮件服务器信息
     * @param title title
     * @param content content
     * @param receiver receiver
     * @return int
     */
    private static int sentSingleEmail(RemoteNotifyServer remoteNotifyServer, String title, String content,
            String receiver) {
        // smtp配置，保存到props文件，读取
        Properties props = generateProperties(remoteNotifyServer);
        // 创建会话
        Session session = getSession(remoteNotifyServer, props);
        MimeMessage msg = new MimeMessage(session);
        try {
            msg.setFrom(remoteNotifyServer.getEmailFrom());
            msg.setSubject(title, EMAIL_CHARSET); // 邮件主题
            msg.setText(content, EMAIL_CHARSET); // 邮件正文
            msg.setRecipients(Message.RecipientType.TO, new Address[]{new InternetAddress(receiver)});
            initMailcapCommandMap();
            Transport.send(msg);
        } catch (MessagingException e) {
            writeExceptionLog(remoteNotifyServer, e);
            return processEmailException(e);
        }
        // 必须把系统变量清除
        recordSuccessLog(remoteNotifyServer);
        return EmailCodes.SUCCESS;
    }

    /**
     * 发送邮件（带附件）
     *
     * @param remoteNotifyServer 邮件服务器信息
     * @param title title
     * @param content content
     * @param receiver receiver
     * @param attachments 附件
     * @return int
     */
    private static int sendSingleAttachmentEmail(RemoteNotifyServer remoteNotifyServer, String title, String content,
            String receiver, String[] attachments) {
        // smtp配置，保存到props文件，读取
        Properties props = generateProperties(remoteNotifyServer);
        // 创建会话
        Session session = getSession(remoteNotifyServer, props);
        MimeMessage msg = new MimeMessage(session);
        try {
            msg.setFrom(remoteNotifyServer.getEmailFrom());
            msg.setSubject(title, EMAIL_CHARSET); // 邮件主题
            msg.setRecipients(Message.RecipientType.TO, new Address[]{new InternetAddress(receiver)});
            initMailcapCommandMap();
            MimeMultipart mp = new MimeMultipart();
            // 正文
            MimeBodyPart text = new MimeBodyPart();
            text.setContent(content, "text/html;charset=" + EMAIL_CHARSET);
            mp.addBodyPart(text);
            // 附件
            for (String path : attachments) {
                File file = new File(path);
                MimeBodyPart attachment = new MimeBodyPart();
                attachment.attachFile(file);
                // 手动指定附件编码为UTF-8 以及文件格式为zip
                String encodedFileName = MimeUtility.encodeText(file.getName(), "UTF-8", null);
                attachment.setFileName(encodedFileName);
                // 手动设置 Content-Type 和 Content-Disposition
                attachment.setHeader("Content-Type",
                    Files.probeContentType(file.toPath()) + "; name=\"" + encodedFileName + "\"");
                attachment.setHeader("Content-Disposition", "attachment; filename=\"" + encodedFileName + "\"");
                mp.addBodyPart(attachment);
            }
            msg.setContent(mp);
            Transport.send(msg);
        } catch (MessagingException | IOException e) {
            writeExceptionLog(remoteNotifyServer, e);
            return processEmailException(e);
        }
        // 必须把系统变量清除
        recordSuccessLog(remoteNotifyServer);
        return EmailCodes.SUCCESS;
    }

    private static Session getSession(RemoteNotifyServer remoteNotifyServer, Properties props) {
        return Session.getInstance(props, new Authenticator() {
            @Override
            protected PasswordAuthentication getPasswordAuthentication() {
                if (Boolean.parseBoolean(props.getProperty(EmailConstants.MAIL_SMTP_AUTH))) {
                    // 需要认证
                    String pwd = remoteNotifyServer.getPassword();
                    pwd = pwd == null ? StringUtils.EMPTY : pwd;
                    return new PasswordAuthentication(remoteNotifyServer.getUserName(), pwd);
                }
                return super.getPasswordAuthentication();
            }
        });
    }

    private static Properties generateProperties(RemoteNotifyServer remoteNotifyServer) {
        Properties props = new Properties();
        if (remoteNotifyServer == null) {
            throw new LegoCheckedException(CommonErrorCode.ALARM_SMTP_CONNECT_FAILED,
                    "The remoteNotifyServer is not deployed.");
        }
        props.put(EmailConstants.MAIL_HOST, remoteNotifyServer.getServer());
        props.put(EmailConstants.MAIL_PORT, String.valueOf(remoteNotifyServer.getPort()));
        props.put(EmailConstants.CHECK_HOST_NAME, FALSE);
        // 如果需要鉴权，则鉴权
        setAuthProperty(remoteNotifyServer, props);
        // 使用代理发送
        setProxyProperty(remoteNotifyServer, props);
        // 如果没有添加证书，则可以直接进行非加密发送邮件，如果添加了相应的证书，则需要走加密通道
        setTLSProperty(remoteNotifyServer, props);
        setSSLProperty(remoteNotifyServer, props);
        // 设置接收超时时间
        props.setProperty(EmailConstants.MAIL_SNMTP_TIME_OUT, TIME_OUT);
        // 设置读取超时时间
        props.setProperty(EmailConstants.MAIL_SMTP_CONNECT_TIME_OUT, TIME_OUT);
        // 设置写入超时时间
        props.setProperty(EmailConstants.MAIL_SMTP_WRITE_TIME_OUT, TIME_OUT);
        log.debug("setProperty: completed");
        return props;
    }

    private static void setProxyProperty(RemoteNotifyServer remoteNotifyServer, Properties props) {
        if (remoteNotifyServer.isProxyEnable()) {
            Properties properties = System.getProperties();
            properties.setProperty(PROXY_SET, TRUE);
            properties.setProperty(SOCKS_PROXY_HOST, remoteNotifyServer.getProxyServer());
            properties.setProperty(SOCKS_PROXY_PORT, String.valueOf(remoteNotifyServer.getProxyPort()));
        } else {
            // 不使用代理
            props.setProperty(PROXY_SET, FALSE);
            props.setProperty(SOCKS_PROXY_HOST, StringUtils.EMPTY);
            props.setProperty(SOCKS_PROXY_PORT, StringUtils.EMPTY);
        }
    }

    /**
     * TLS方式需要的属性
     *
     * @param remoteNotifyServer 邮件服务器设置
     * @param props 属性
     */
    private static void setTLSProperty(RemoteNotifyServer remoteNotifyServer, Properties props) {
        if (remoteNotifyServer.getIsTlsEnable() != null && remoteNotifyServer.getIsTlsEnable()) {
            // 启用TLS
            props.setProperty(EmailConstants.MAIL_TRANSPORT_STARTTLS_ENABLE, TRUE);
            props.put(EmailConstants.STARTTLS_REQUIRED, TRUE);
            props.put(EmailConstants.MAIL_PORT, String.valueOf(remoteNotifyServer.getPort()));
            props.setProperty(SmtpSslSocketFactory.SMTP_IP_ADDRESS, remoteNotifyServer.getServer());
            props.setProperty(SmtpSslSocketFactory.SMTP_SSL_CONTEXT, SecurityUtil.getTlsContext());
            props.put(EmailConstants.MAIL_SMTP_SSL_SOCKET_FACTORY, new SmtpSslSocketFactory(props));
            props.setProperty(EmailConstants.MAIL_SMTP_SSL_SOCKET_FALLBACK, FALSE);
            props.setProperty(EmailConstants.MAIL_TRANSPORT_PROTOCOL, EmailConstants.SMTP_PROTOCOL);
            props.setProperty(EmailConstants.MAIL_SMTP_SENDPARTIAL, FALSE);
            props.setProperty(EmailConstants.MAIL_SMTPS_SENDPARTIAL, FALSE);
            // 是否开启证书CA校验
            log.info("TLS Indicates whether to enable CA verification: {}", remoteNotifyServer.getIsContainCert());
            if (!remoteNotifyServer.getIsContainCert()) {
                setNoCertificate(props, SecurityUtil.getTlsContext());
            }
        }
    }

    private static void setSSLProperty(RemoteNotifyServer remoteNotifyServer, Properties props) {
        if (remoteNotifyServer.getIsSslEnable() != null && remoteNotifyServer.getIsSslEnable()) {
            // 启用SSL
            props.setProperty(EmailConstants.MAIL_SMTP_SSL_ENABLE, TRUE);
            props.setProperty(EmailConstants.MAIL_PORT, String.valueOf(remoteNotifyServer.getSslSmtpPort()));
            props.setProperty(SmtpSslSocketFactory.SMTP_IP_ADDRESS, remoteNotifyServer.getServer());
            props.put(EmailConstants.MAIL_SMTP_SSL_SOCKET_FACTORY, new SmtpSslSocketFactory(props));
            props.setProperty(EmailConstants.MAIL_SMTP_SSL_SOCKET_FALLBACK, FALSE);
            props.setProperty(EmailConstants.MAIL_SMTP_SSL_SOCKET_FACTORY_PORT,
                    String.valueOf(remoteNotifyServer.getSslSmtpPort()));
            props.setProperty(SmtpSslSocketFactory.SMTP_SSL_CONTEXT, SecurityUtil.getSslContext());
            // 是否开启证书CA校验
            log.info("SSL Indicates whether to enable CA verification: {}", remoteNotifyServer.getIsContainCert());
            if (!remoteNotifyServer.getIsContainCert()) {
                setNoCertificate(props, SecurityUtil.getSslContext());
            }
        }
    }

    private static void setNoCertificate(Properties props, String type) {
        TrustManager[] trustAllCerts = new TrustManager[]{new X509TrustManager() {
            /**
             * 授信
             *
             * @return X509Certificate
             */
            public java.security.cert.X509Certificate[] getAcceptedIssuers() {
                return new java.security.cert.X509Certificate[0]; // 返回空数组
            }

            /**
             * 检查受信任的客户端
             *
             * @param certs the peer certificate chain
             * @param authType the authentication type based on the client certificate
             */
            public void checkClientTrusted(java.security.cert.X509Certificate[] certs, String authType) {
                log.info("Check Client Trusted");
            }

            /**
             * 检查受信任的服务器
             *
             * @param certs the peer certificate chain
             * @param authType the key exchange algorithm used
             */
            public void checkServerTrusted(java.security.cert.X509Certificate[] certs, String authType) {
                log.info("Check Server Trusted");
            }
        }};
        SSLContext sc;
        try {
            sc = SSLContext.getInstance(type);
            sc.init(null, trustAllCerts, new java.security.SecureRandom());
        } catch (NoSuchAlgorithmException | KeyManagementException e) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Mail Service Issues");
        }
        SSLSocketFactory sslSocketFactory = sc.getSocketFactory();
        props.put("mail.smtp.ssl.socketFactory", sslSocketFactory);
    }

    private static void setAuthProperty(RemoteNotifyServer remoteNotifyServer, Properties props) {
        if (remoteNotifyServer.isValidateEnable()) {
            props.put(EmailConstants.MAIL_SMTP_AUTH, TRUE);
        }
    }

    private static void initMailcapCommandMap() {
        MailcapCommandMap mailcapCommandMap = new MailcapCommandMap();
        if (CommandMap.getDefaultCommandMap() instanceof MailcapCommandMap) {
            mailcapCommandMap = (MailcapCommandMap) CommandMap.getDefaultCommandMap();
        }
        mailcapCommandMap.addMailcap("text/html;; x-java-content-handler=com.sun.mail.handlers.text_html");
        mailcapCommandMap.addMailcap("text/xml;; x-java-content-handler=com.sun.mail.handlers.text_xml");
        mailcapCommandMap.addMailcap("text/plain;; x-java-content-handler=com.sun.mail.handlers.text_plain");
        mailcapCommandMap.addMailcap("multipart/*;; x-java-content-handler=com.sun.mail.handlers.multipart_mixed");
        mailcapCommandMap.addMailcap("message/rfc822;; x-java-content-handler=com.sun.mail.handlers.message_rfc822");
        CommandMap.setDefaultCommandMap(mailcapCommandMap);
    }

    private static void writeExceptionLog(RemoteNotifyServer remoteNotifyServer, Throwable throwable) {
        log.error(
                "Sending the receiver the following server failed, "
                        + "mail server is: {}, port is: {}, proxy server is: {}, proxy port is: {}.",
                remoteNotifyServer.getServer(), remoteNotifyServer.getPort(), remoteNotifyServer.getProxyServer(),
                remoteNotifyServer.getProxyPort(), throwable);
    }

    private static void recordSuccessLog(RemoteNotifyServer remoteNotifyServer) {
        log.info(
                "Sending the receiver the following server success, mail server is: {}, "
                        + "port is: {}, proxy server is: {}, proxy port is: {}.",
                remoteNotifyServer.getServer(), remoteNotifyServer.getPort(), remoteNotifyServer.getProxyServer(),
                remoteNotifyServer.getProxyPort());
    }

    /**
     * 处理邮件异常
     *
     * @param exception e
     * @return int
     */
    private static int processEmailException(Exception exception) {
        if (exception instanceof MessagingException) {
            return processMessagingException(exception);
        }
        return EmailCodes.ERROR_CODE_EXCEPTION;
    }

    private static int processMessagingException(Exception exception) {
        if (exception.getCause() != null && (ExceptionUtil.lookFor(exception, CertificateException.class) != null
                || ExceptionUtil.lookFor(exception, SSLHandshakeException.class) != null)) {
            return EmailCodes.SMTP_CERTIFICATE_VERIFICATION_FAILED;
        } else if (exception.getCause() != null && exception.getMessage().contains("Can't verify identity of server")) {
            return EmailCodes.SMTP_SERVER_IDENTITY_CHECK_FAIL;
        } else if (isAuthenticationFailed(exception)) {
            // 邮件身份认证失败
            return EmailCodes.FAILED_AUTHENTICATE;
        } else if (containsMessage(exception, "Can't connect to SOCKS proxy")) {
            // 连接代理服务器失败
            log.info("Can't connect to SOCKS proxy");
            return EmailCodes.CONNECT_PROXY_FAILED;
        } else if (isEncryptProtocolError(exception)) {
            return EmailCodes.WORRY_ENCRYPT_PROTOCOL;
        } else if (containsMessage(exception, "Invalid Addresses")) {
            // 非法地址
            return EmailCodes.INVALID_EMAIL_ADDRESS;
        } else if (isConnectFailException(exception)) {
            return EmailCodes.EMAIL_TIME_OUT;
        } else {
            // 连接异常
            return EmailCodes.ERROR_CODE_EXCEPTION;
        }
    }

    private static boolean isAuthenticationFailed(Exception exception) {
        if (exception.getCause() instanceof AuthenticationFailedException) {
            return true;
        }
        return containsMessage(exception, "authentication is required");
    }

    private static boolean containsMessage(Exception exception, String message) {
        List<Throwable> throwableList = ExceptionUtils.getThrowableList(exception);
        return throwableList.stream().anyMatch(throwable -> throwable.getMessage().contains(message));
    }

    private static boolean isEncryptProtocolError(Exception exception) {
        // 接口不需要加密算法，但是加密配置了STARTTLS加密
        if (containsMessage(exception, "STARTTLS is required")) {
            return true;
        }
        // 接口配置需要STARTTLS加密，但是加密方式配置的是不加密
        if (containsMessage(exception, "220 Ready to start TLS")) {
            return true;
        }
        // 接口配置需要SSL/TLS加密，但是加密算法配置错误场景
        if (containsMessage(exception, "Read timed out")) {
            return true;
        }
        // 接口不需要加密或者STARTTLS算法，配置了SSL/TLS加密时
        return containsMessage(exception, "Unrecognized SSL message");
    }

    private static boolean isConnectFailException(Exception exception) {
        if (containsMessage(exception, "connect timed out")) {
            return true;
        }
        // 没有相关服务
        if (containsMessage(exception, "Connection refused")) {
            return true;
        }
        // 有IP的网卡，但是IP不存在
        return containsMessage(exception, "Host unreachable");
    }
}
