package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 查询hcs资源集的参数
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/7/27
 */
@Getter
@Setter
public class QueryHcsProjectsParam {
    private String username;
    private String password;
    private int clusterId;
    private String tenantName;
    private String tenantId;
}
