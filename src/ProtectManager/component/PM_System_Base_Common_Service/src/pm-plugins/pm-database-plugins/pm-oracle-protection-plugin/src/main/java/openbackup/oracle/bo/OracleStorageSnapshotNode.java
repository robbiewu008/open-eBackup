package openbackup.oracle.bo;

import openbackup.data.protection.access.provider.sdk.base.Authentication;

import lombok.Getter;
import lombok.Setter;

import java.util.Map;

/**
 * 功能描述
 *
 * @author x00464136
 * @since 2024-01-09
 */
@Getter
@Setter
public class OracleStorageSnapshotNode {
    /**
     * 资源ID
     */
    private String id;

    /**
     * 资源名称
     */
    private String name;

    /**
     * 资源类型
     */
    private String type;

    /**
     * 资源子类型
     */
    private String subType;

    /**
     * 访问地址，ip或者域名
     */
    private String endpoint;

    /**
     * 端口
     */
    private Integer port;

    /**
     * 环境的认证信息
     */
    private Authentication auth;

    /**
     * 资源的扩展信息
     */
    private Map<String, String> extendInfo;
}
