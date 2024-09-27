package openbackup.system.base.common.rest;

import openbackup.system.base.common.scurity.IBcmX509TrustHandler;
import openbackup.system.base.common.utils.ExceptionUtil;

import lombok.extern.slf4j.Slf4j;

import java.security.cert.CertificateException;

/**
 * SSL证书校验handler类
 *
 * @author p30001902
 * @since 2021-05-11
 */
@Slf4j
public class CommonX509TrustHandler implements IBcmX509TrustHandler {
    /**
     * handle error
     *
     * @param phase 错误码
     */
    @Override
    public void handle(long phase) {}

    /**
     * handle certificate verification exceptions
     *
     * @param exception received exceptions
     * @throws CertificateException 认证异常
     */
    @Override
    public void handle(Exception exception) throws CertificateException {
        if (exception == null) {
            return;
        }
        // 证书校验失败的话，就抛出
        if (exception instanceof CertificateException) {
            throw (CertificateException) exception;
        }
        log.error("Unexpected exception happen in certificate verify.", ExceptionUtil.getErrorMessage(exception));
    }
}
