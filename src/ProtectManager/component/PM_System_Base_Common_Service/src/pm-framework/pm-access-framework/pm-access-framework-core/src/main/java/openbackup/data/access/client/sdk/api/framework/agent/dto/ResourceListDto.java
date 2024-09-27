package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * PM向Agent查询应用详细信息(V2接口)返回体
 *
 * @author 30009433
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-19
 */
@Setter
@Getter
public class ResourceListDto {
    private int pageNo;

    private int pageSize;

    private int pages;

    private int total;

    private List<AppResource> items;
}
