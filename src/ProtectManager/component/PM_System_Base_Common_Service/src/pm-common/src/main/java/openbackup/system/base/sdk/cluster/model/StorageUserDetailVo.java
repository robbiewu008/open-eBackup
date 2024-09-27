package openbackup.system.base.sdk.cluster.model;

import lombok.Data;
import lombok.ToString;

/**
 * 查询本地备份介质的账号密码信息
 *
 * @author l00853347
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-07-18
 */
@Data
@ToString(exclude = {"password"})
public class StorageUserDetailVo {
    private String esn;

    private String linkIp;

    private Integer port;

    private String managementIps;

    private String username;

    private String password;

    private String deviceType;
}
