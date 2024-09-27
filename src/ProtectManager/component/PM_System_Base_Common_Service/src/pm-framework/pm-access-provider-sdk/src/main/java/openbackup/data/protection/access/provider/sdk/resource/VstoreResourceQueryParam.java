package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Builder;
import lombok.Data;

/**
 * 资源查询参数
 *
 * @author c00642388
 * @since 2024-04-25
 */
@Builder
@Data
public class VstoreResourceQueryParam {
    private int page;
    private int size;
    private boolean isSearchProtectObject;
    private String key;
    private String order;
    private String vstoreNameFilter;
    private String orderByFsNum;
}
