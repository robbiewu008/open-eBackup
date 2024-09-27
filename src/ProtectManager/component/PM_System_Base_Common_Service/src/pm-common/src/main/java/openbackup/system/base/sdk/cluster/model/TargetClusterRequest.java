package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.common.constants.IsmConstant;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * 目标集群请求下发基类
 *
 * @author p30001902
 * @since 2020-06-11
 */
@Data
public class TargetClusterRequest {
    @NotBlank(message = "The clusterName cannot be blank. ")
    @Length(max = IsmNumberConstant.TWO_HUNDRED_FIFTY_SIX, min = IsmNumberConstant.FOUR, message = "The length of cluster name is 4-256 characters")
    @Pattern(regexp = RegexpConstants.NAME_STR, message = "cluster name is invalid")
    private String clusterName;

    @NotBlank(message = "The IP of cluster cannot be blank")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "cluster ip is invalid, not ipv4 or ipv6.")
    private String ip;

    @NotNull(message = "The port cannot be null")
    @Max(IsmConstant.PORT_MAX)
    @Min(IsmConstant.PORT_MIN)
    private Integer port;

    @NotBlank(message = "The username cannot be blank. ")
    @Length(min = IsmNumberConstant.ONE, max = IsmNumberConstant.SIXTY_FOUR)
    private String username;

    // 修改时可能不会下发密码，不在这里校验
    private String password;

    @NotNull(message = "The role cannot be null")
    @Min(IsmNumberConstant.ZERO)
    @Max(IsmNumberConstant.TWO)
    private Integer role;

    // 默认会转发到其他管理集群进行同步修改、添加
    @JsonProperty("syncToRemote")
    private boolean shouldSyncToRemote = true;
}
