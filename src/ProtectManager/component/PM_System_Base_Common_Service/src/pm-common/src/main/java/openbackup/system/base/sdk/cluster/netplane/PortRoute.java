package openbackup.system.base.sdk.cluster.netplane;

import openbackup.system.base.common.utils.network.AddressUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.common.validator.constants.ValidateGroups;
import openbackup.system.base.sdk.cluster.netplane.validateprovider.PortRouteGroupProvider;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.group.GroupSequenceProvider;

import javax.validation.constraints.NotBlank;
import javax.validation.constraints.Pattern;

/**
 * PortRoute
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-03-28
 */
@Getter
@Setter
@GroupSequenceProvider(PortRouteGroupProvider.class)
public class PortRoute {
    /**
     * 路由类型
     */
    @Pattern(regexp = "[012]")
    @NotBlank
    private String routeType;

    /**
     * 目标地址
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4_WITH_DEFAULT, message = "value invalid",
        groups = {ValidateGroups.IPv4Group.class})
    @Pattern(regexp = AddressUtil.IPV6REG, message = "value invalid",
        groups = {ValidateGroups.IPv6Group.class})
    @Pattern(regexp = "(0\\.0\\.0\\.0)|(::)", groups = {ValidateGroups.DefaultRoute.class})
    @NotBlank
    private String destination;

    /**
     * IPv4/IPv6网关
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4_WITH_DEFAULT, message = "value invalid",
        groups = {ValidateGroups.IPv4Group.class})
    @Pattern(regexp = AddressUtil.IPV6REG, message = "value invalid",
        groups = {ValidateGroups.IPv6Group.class})
    @NotBlank
    private String gateway;

    /**
     * 目的掩码
     */
    @NotBlank
    @Pattern(regexp = RegexpConstants.IPV4_SUB_NETMASK, message = "value invalid",
        groups = {ValidateGroups.IPv4Group.class})
    @Pattern(regexp = RegexpConstants.IPV6_SUB_NETMASK, message = "value invalid",
        groups = {ValidateGroups.IPv6Group.class})
    @Pattern(regexp = "(0\\.0\\.0\\.0)|(0)", groups = {ValidateGroups.DefaultRoute.class})
    @Pattern(regexp = "(255\\.255\\.255\\.255)|(128)", groups = {ValidateGroups.MasterRoute.class})
    private String mask;
}
