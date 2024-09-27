package openbackup.system.base.sdk.cluster.request;

import lombok.Getter;
import lombok.Setter;

import org.springframework.web.multipart.MultipartFile;

/**
 * 证书替换对象
 *
 * @author fwx1022842
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/6/27
 */
@Getter
@Setter
public class CertReplaceRequest {
    /**
     * ca证书
     */
    MultipartFile caCertificate;

    /**
     * 服务端证书
     */
    MultipartFile serverCertificate;

    /**
     * 服务端私钥
     */
    MultipartFile serverKey;

    /**
     * keystore
     */
    MultipartFile keystore;
}