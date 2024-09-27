package openbackup.system.base.sdk.cluster.request;

import lombok.Getter;
import lombok.Setter;

import org.springframework.web.multipart.MultipartFile;

/**
 * HaCertRequest
 *
 * @author fwx1022842
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/9/15
 */
@Setter
@Getter
public class HaCertRequest {
    /**
     * Ha Ca证书
     */
    MultipartFile caCert;

    /**
     * Ha Ca私钥
     */
    MultipartFile caPem;

    /**
     * Ha Ca私钥密码
     */
    MultipartFile caPwd;

    /**
     * Ha server证书
     */
    MultipartFile serverCert;

    /**
     * Ha server私钥
     */
    MultipartFile serverPem;

    /**
     * Ha server私钥密码
     */
    MultipartFile serverPwd;

    /**
     * 更新前的Ha Ca证书
     */
    MultipartFile oldCaCert;

    /**
     * 更新前的Ha Ca私钥
     */
    MultipartFile oldCaPem;

    /**
     * 更新前的Ha Ca私钥密码
     */
    MultipartFile oldCaPwd;

    /**
     * 更新前的Ha server证书
     */
    MultipartFile oldServerCert;

    /**
     * 更新前的Ha server私钥
     */
    MultipartFile oldServerPem;

    /**
     * 更新前的Ha server私钥密码
     */
    MultipartFile oldServerPwd;
}
