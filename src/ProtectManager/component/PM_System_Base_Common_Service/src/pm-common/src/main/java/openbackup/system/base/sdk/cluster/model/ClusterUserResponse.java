package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

import java.util.List;

/**
 * ClusterUserResponse
 *
 * @author dWX1009286
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-02-24
 */
@Data
public class ClusterUserResponse<T> {
    // 用户列表
    private List<T> userList;
}
