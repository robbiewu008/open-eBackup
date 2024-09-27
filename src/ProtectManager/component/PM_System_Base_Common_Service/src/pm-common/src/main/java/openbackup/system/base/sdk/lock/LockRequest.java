package openbackup.system.base.sdk.lock;

import lombok.Data;

import java.util.List;

/**
 * 创建资源锁请求
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/1/26
 **/
@Data
public class LockRequest {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 资源锁id
     */
    private String lockId;

    /**
     * 需要锁定的资源列表
     */
    private List<LockResource> resources;

    /**
     * 资源锁优先级
     */
    private int priority = 3;
}
