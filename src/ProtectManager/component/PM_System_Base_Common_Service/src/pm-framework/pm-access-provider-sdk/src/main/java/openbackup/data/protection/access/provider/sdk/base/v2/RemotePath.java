package openbackup.data.protection.access.provider.sdk.base.v2;

import lombok.Data;

/**
 * DME副本信息中存储仓路径信息
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-11
 */
@Data
public class RemotePath {
    /**
     * 类型：0-元数据；1-普通数据
     */
    private int type;

    /**
     * 具体存储路径
     */
    private String path;

    /**
     * 文件系统 ID
     */
    private String id;

    /**
     * 租户ID
     */
    private String parentId;
}
