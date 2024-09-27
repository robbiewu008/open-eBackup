package openbackup.access.framework.resource.client.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 资源信息请求体
 *
 * @author x30028756
 * @since 2023-7-4
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class ResourceInfo {
    /**
     * 文件系统ID
     */
    private String resourceId;

    /**
     * 文件系统名称
     */
    private String fsName;

    /**
     * 租户ID
     */
    private String vstoreId;

    /**
     * 租户名称
     */
    private String vstoreName;

    /**
     * 文件系统创建时间
     */
    private String createdTime;
}
