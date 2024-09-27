package openbackup.data.protection.access.provider.sdk.resourcegroup.req;

import lombok.Getter;
import lombok.Setter;

/**
 * 查询资源组里列表请求体
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-01-23
 */

@Getter
@Setter
public class ResourceGroupQueryParams {
    private String conditions;
    private Integer pageNo;
    private Integer pageSize;
    private String orders;
}