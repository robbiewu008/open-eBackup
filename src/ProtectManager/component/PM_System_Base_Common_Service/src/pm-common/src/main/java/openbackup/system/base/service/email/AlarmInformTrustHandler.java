package openbackup.system.base.service.email;

import openbackup.system.base.common.scurity.IBcmX509TrustHandler;
import openbackup.system.base.common.utils.ExceptionUtil;

import lombok.extern.slf4j.Slf4j;

import java.security.cert.CertificateException;

/**
 * 告警通知的证书异常处理器
 *
 * @author g30003063
 * @since 2021-05-10
 */
@Slf4j
public class AlarmInformTrustHandler implements IBcmX509TrustHandler {
    @Override
    public void handle(long phase) {
        // BCManager用于生成告警的，暂不删除
    }

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
