package openbackup.system.base.common.scurity;

import java.security.cert.CertificateException;

/**
 * 用于验证服务器证书的TrustManager
 *
 * @author l90005176
 * @version [OceanStor BCManager V200R001C00, 2016年2月16日]
 * @since 2019-10-25
 */
public interface IBcmX509TrustHandler {
    /**
     * 处理错误码
     *
     * @param phase 错误码
     */
    void handle(long phase);

    /**
     * 处理错误
     *
     * @param exception 异常
     * @throws CertificateException 证书校验失败异常
     */
    void handle(Exception exception) throws CertificateException;
}
