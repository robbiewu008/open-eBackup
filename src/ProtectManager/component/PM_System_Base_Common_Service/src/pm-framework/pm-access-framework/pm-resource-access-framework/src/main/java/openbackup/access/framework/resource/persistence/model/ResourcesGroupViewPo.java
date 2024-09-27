package openbackup.access.framework.resource.persistence.model;

import lombok.Data;

/**
 * 分组查询对象
 *
 * @author t30028453
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-21
 */
@Data
public class ResourcesGroupViewPo {
    /**
     * value值
     */
    private String value;

    /**
     * 以逗号拼接的资源id
     */
    private String resourceIds;
}
