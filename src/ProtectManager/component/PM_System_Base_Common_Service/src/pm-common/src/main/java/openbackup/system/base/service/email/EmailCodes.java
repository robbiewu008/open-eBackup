/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.service.email;

/**
 * 邮件发送错误码
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-14
 */
public class EmailCodes {
    /**
     * 发送异常
     */
    public static final int ERROR_CODE_EXCEPTION = -1;

    /**
     * 发送成功
     */
    public static final int SUCCESS = 0;

    /**
     * 发送超时
     */
    public static final int EMAIL_TIME_OUT = 6;

    /**
     * 非法的邮件地址。
     */
    public static final int INVALID_EMAIL_ADDRESS = 7;

    /**
     * 连接代理服务器失败
     */
    public static final int CONNECT_PROXY_FAILED = 8;

    /**
     * 发件人邮箱为空
     */
    public static final int EMPTY_SENDER_ADDRESS = 10;

    /**
     * 邮件服务器为空
     */
    public static final int EMPTY_EMAIL_SERVER = 11;

    /**
     * 邮件服务器身份认证失败
     */
    public static final int FAILED_AUTHENTICATE = 12;

    /**
     * smt服务器加密协议不一致
     */
    public static final int WORRY_ENCRYPT_PROTOCOL = 13;

    /**
     * 该SMTP服务器的SSL证书不合法。
     */
    public static final int SMTP_SERVER_IDENTITY_CHECK_FAIL = 14;

    /**
     * 未导入CA证书或者导入CA证书校验失败。
     */
    public static final int SMTP_CERTIFICATE_VERIFICATION_FAILED = 15;
}
