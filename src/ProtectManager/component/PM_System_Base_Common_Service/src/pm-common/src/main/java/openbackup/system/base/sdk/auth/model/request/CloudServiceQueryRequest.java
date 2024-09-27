package openbackup.system.base.sdk.auth.model.request;

import lombok.Getter;
import lombok.Setter;

/**
 * 云服务参数 查询参数
 *
 * @author y30021475
 * @since 2023-07-28
 */
@Getter
@Setter
public class CloudServiceQueryRequest {
    private String regionCode;

    private String indexNames;

    private int pageNo;

    private int pageSize;
}
