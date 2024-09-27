package openbackup.access.framework.resource.dto;

import openbackup.data.protection.access.provider.sdk.base.Authentication;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import lombok.EqualsAndHashCode;

import org.hibernate.validator.constraints.Length;

/**
 * 受保护环境DTO对象
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-14
 */
@EqualsAndHashCode(callSuper = true)
@Data
public class ProtectedEnvironmentDto extends BaseProtectedEnvironmentDto {
    // 资源类型（主类）
    @Length(max = 64)
    private String type;

    // 资源子类
    @Length(max = 64)
    private String subType;

    // 版本
    @Length(max = 64)
    private String version;

    // 创建时间
    @Length(max = 256)
    private String createdTime;

    // 用户id
    @Length(max = 255)
    @JsonProperty("userid")
    private String userId;

    // 是否开启多组户共享
    private Boolean isShared;

    // 1，注册安装，2，更新
    private String registerType;

    // 认证信息
    private Authentication auth;
}
