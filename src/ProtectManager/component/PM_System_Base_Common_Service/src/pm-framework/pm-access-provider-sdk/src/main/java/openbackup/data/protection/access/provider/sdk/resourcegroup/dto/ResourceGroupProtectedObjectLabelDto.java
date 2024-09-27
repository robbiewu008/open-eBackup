package openbackup.data.protection.access.provider.sdk.resourcegroup.dto;

import lombok.Getter;
import lombok.Setter;

/**
 * ResourceGroupProtectedObjectLabelDto
 *
 * @author l30046868
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-04-07
 */
@Getter
@Setter
public class ResourceGroupProtectedObjectLabelDto {
    /* 资源组名称 */
    private String name;

    /* 后置操作标记 */
    private String label;
}
