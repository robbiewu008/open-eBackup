package openbackup.access.framework.resource.persistence.model;

import lombok.Data;

import java.util.Map;

/**
 * 资源repository查询参数
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-23
 */
@Data
public class ResourceRepositoryQueryParams {
    // 查询条件是否忽略资源的拥有者
    private boolean shouldIgnoreOwner;

    private boolean isDesesitization;

    private int page;

    private int size;

    private Map<String, Object> conditions;

    private String[] orders;

    public ResourceRepositoryQueryParams() {
        shouldIgnoreOwner = false;
    }

    public ResourceRepositoryQueryParams(boolean shouldIgnoreOwner, int page, int size, Map<String, Object> conditions,
        String[] orders) {
        this.shouldIgnoreOwner = shouldIgnoreOwner;
        this.page = page;
        this.size = size;
        this.conditions = conditions;
        this.orders = orders;
    }
}
