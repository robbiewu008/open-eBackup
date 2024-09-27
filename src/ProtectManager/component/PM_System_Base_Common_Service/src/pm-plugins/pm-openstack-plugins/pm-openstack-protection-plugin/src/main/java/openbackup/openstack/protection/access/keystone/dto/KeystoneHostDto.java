package openbackup.openstack.protection.access.keystone.dto;

import lombok.Getter;
import lombok.Setter;

/**
 * keystone主机信息
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-15
 */
@Getter
@Setter
public class KeystoneHostDto {
    /**
     * 注册到keystone的请求url，域名部分会被转化为ip
     */
    private String keystoneUrl;

    /**
     * 注册到keystone的请求头参数，转化url的域名部分
     */
    private String host;
}
