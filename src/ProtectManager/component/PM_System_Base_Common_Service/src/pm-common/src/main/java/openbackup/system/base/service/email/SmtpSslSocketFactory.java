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

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.scurity.BcmX509TrustManager;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.SecurityUtil;

import org.springframework.util.StringUtils;

import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Properties;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;

/**
 * SMTP SSL Socket工厂
 *
 */
@Slf4j
public class SmtpSslSocketFactory extends BcmSslSocketFactory {
    /**
     * smtp 调用的IP地址
     */
    public static final String SMTP_IP_ADDRESS = "smtpIpAddress";

    /**
     * smtp加密方法
     */
    public static final String SMTP_SSL_CONTEXT = "smtpSSLContext";

    private final Properties props;

    /**
     * 构造方法，给父类的构造方法提供参数，支持TLSv1.2 TLSv1.3连接
     *
     * @param props Ip地址
     */
    public SmtpSslSocketFactory(Properties props) {
        super(props.getProperty(SMTP_IP_ADDRESS), SecurityUtil.getProtocols());
        this.props = props;
    }

    /**
     * 创建SSLSocketFactory
     *
     * @return SSLSocketFactory SSLSocketFactory
     */
    @Override
    public SSLSocketFactory createSslSocketFactory() {
        SSLSocketFactory sslSocketFactory;
        try {
            String sslContext = props.getProperty(SmtpSslSocketFactory.SMTP_SSL_CONTEXT);
            if (StringUtils.isEmpty(sslContext)) {
                throw new IllegalArgumentException("SSL context should not be empty.");
            }
            SSLContext context = SSLContext.getInstance(sslContext);

            final String ipAddress = props.getProperty(SMTP_IP_ADDRESS);
            context.init(null, new TrustManager[] {new BcmX509TrustManager(ipAddress, new AlarmInformTrustHandler())},
                SecureRandom.getInstanceStrong());
            sslSocketFactory = context.getSocketFactory();
        } catch (NoSuchAlgorithmException | KeyManagementException e) {
            log.error("Creating SSLSocketFactory failed, IP address: {}.", props.getProperty(SMTP_IP_ADDRESS),
                ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(ErrorCodeConstant.SSL_INIT_OR_CONNECT_FAIL,
                "Initializing BcmSslSocketFactory failed.", ExceptionUtil.getErrorMessage(e));
        }
        return sslSocketFactory;
    }
}
