package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Data;

import java.util.List;

/**
 * 根据key分组查询资源列表
 *
 * @author t30028453
 * @version [X8000 1.2.1]
 * @since 2022-05-20
 */
@Data
public class ProtectedResourceGroupResult {
    /**
     * 扩展属性 key 名称
     */
    private String key;

    /**
     * 扩展属性 value 值
     */
    private String value;

    /**
     * 资源集合
     */
    private List<ProtectedResource> resources;
}
