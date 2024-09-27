package openbackup.system.base.sdk.devicemanager.request;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * pacific 某节点上的业务网络配置信息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-05
 */
@Getter
@Setter
public class NodeNetworkInfoRequest {
    // 节点管理ip
    @NotNull(message = "The manage ip cannot be null")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "manage ip is invalid, not ipv4 or ipv6.")
    private String manageIp;

    // 业务网络信息
    @Size(min = 1)
    @NotNull(message = "The ipInfoList cannot be null.")
    private List<IpInfo> ipInfoList;
}
