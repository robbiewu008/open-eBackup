package openbackup.system.base.sdk.cert;

import java.io.File;

/**
 * 获取证书服务
 *
 * @author w30042425
 * @since 2023-01-28
 */
public interface CertInternalService {
    /**
     * 根据证书类型获取证书
     *
     * @param type 类型
     * @return 证书文件
     */
    File[] getFirstCertByType(int type);
}
