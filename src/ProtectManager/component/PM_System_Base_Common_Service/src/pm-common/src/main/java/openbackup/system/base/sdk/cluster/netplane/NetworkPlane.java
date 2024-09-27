package openbackup.system.base.sdk.cluster.netplane;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 网络平面 成员信息（来源：DM sdk）
 *
 * @author w00607005
 * @since 2023-05-17
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class NetworkPlane {
    /**
     * id
     */
    @JsonProperty("id")
    private String id;

    /**
     * 名称
     */
    @JsonProperty("name")
    private String name;

    /**
     * 交换机的最大传输值
     */
    @JsonProperty("mtu")
    private String mtu;

    /**
     * 虚拟局域网id
     */
    @JsonProperty("vlanid")
    private String vlanid;

    /**
     * 子网库
     */
    @JsonProperty("ipv4SubNetBase")
    private String ipv4SubNetBase;

    /**
     * 子网掩码
     */
    @JsonProperty("ipv4NetMask")
    private String ipv4NetMask;

    /**
     * ipv4的网关地址
     */
    @JsonProperty("ipv4Gateway")
    private String ipv4Gateway;

    /**
     * 子网变动范围
     */
    @JsonProperty("ipv4SubNetRange")
    private String ipv4SubNetRange;

    /**
     * ipv6子网变动范围
     */
    @JsonProperty("ipv6SubNetRange")
    private String ipv6SubNetRange;

    /**
     * ipv6前缀长度
     */
    @JsonProperty("ipv6NetMask")
    private String ipv6NetMask;

    /**
     * ipv6网关地址
     */
    @JsonProperty("ipv6GateWay")
    private String ipv6GateWay;

    /**
     * ipv6子网库
     */
    @JsonProperty("ipv6SubNetBase")
    private String ipv6SubNetBase;

    /**
     * ipv6子网库
     */
    @JsonProperty("failover")
    private String failover;
}
