package openbackup.system.base.sdk.saml;

import org.springframework.web.multipart.MultipartFile;

import javax.servlet.http.HttpServletRequest;

/**
 * Saml服务
 *
 * @author w30042425
 * @version [OceanProtect X800 1.3.0]
 * @since 2023-02-15
 */
public interface SamlInternalService {
    /**
     * 获取saml metadata
     *
     * @param httpServletRequest 请求头
     * @return metadata数据
     */
    String getMarshallMetadata(HttpServletRequest httpServletRequest);

    /**
     * 校验metadata文件是否合法
     *
     * @param multipartFile 上传的文件
     */
    void validateIPDMetadata(MultipartFile multipartFile);
}
