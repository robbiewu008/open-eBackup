package openbackup.system.base.sdk.cluster.model.ha;

import static openbackup.system.base.common.constants.IsmNumberConstant.ONE;
import static openbackup.system.base.common.constants.IsmNumberConstant.THREE;
import static openbackup.system.base.common.constants.IsmNumberConstant.TWO_HUNDRED_FIFTY_SIX;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Data;

import java.util.List;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 校验备节点浮动IP地址与仲裁网关的可用性请求体
 *
 * @author w00607005
 * @since 2023-05-17
 */
@Data
public class CheckStandbyNetworkRequest {
    /**
     * 浮动IP地址
     */
    @NotNull(message = "The floatIpAddress cannot be null. ")
    @Size(max = TWO_HUNDRED_FIFTY_SIX, min = ONE, message = "The length of floatIpAddress is 1-256 characters")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "floatIpAddress is invalid")
    private String floatIpAddress;

    /**
     * 仲裁网关地址
     */
    @NotNull(message = "The gatewayIpList cannot be null. ")
    @Size(max = THREE, min = ONE, message = "The size of gatewayIpList is 1-3")
    private List<String> gatewayIpList;

    /**
     * 主节点部署类型
     */
    @NotNull(message = "The deployType cannot be null. ")
    private String deployType;
}
