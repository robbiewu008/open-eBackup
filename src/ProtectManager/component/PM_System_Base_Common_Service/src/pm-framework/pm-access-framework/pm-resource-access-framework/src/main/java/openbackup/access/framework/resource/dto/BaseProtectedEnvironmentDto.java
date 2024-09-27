package openbackup.access.framework.resource.dto;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import java.util.List;
import java.util.Map;

import javax.validation.constraints.Max;

/**
 * 更新受保护环境信息DTO对象
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-14
 */
@Data
public class BaseProtectedEnvironmentDto {
    // 受保护环境的uuid
    @Length(max = 64)
    private String uuid;

    // 受保护环境的名称
    @Length(max = 64)
    private String name;

    // 受保护环境的IP地址和域名
    @Length(max = 128)
    private String endpoint;

    // 受保护环境的端口
    @Max(65535)
    private int port;

    // 受保护环境的用户名
    @Length(max = 256)
    private String username;

    // 受保护环境的密码
    @Length(max = 2048)
    private String password;

    // 受保护环境的操作系统类型
    @Length(max = 32)
    private String osType;

    /**
     * 资源的扩展属性
     */
    private Map<String, String> extendInfo;

    // 资源的依赖
    private Map<String, List<ProtectedResource>> dependencies;

    /**
     * 认证信息
     */
    private Authentication auth;
}
