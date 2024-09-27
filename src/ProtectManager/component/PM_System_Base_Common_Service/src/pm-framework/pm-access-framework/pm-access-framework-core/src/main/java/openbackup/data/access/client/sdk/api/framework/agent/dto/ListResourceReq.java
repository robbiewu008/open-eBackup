package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

/**
 * PM向Agent查询应用详细信息后的返回对象
 *
 * @author fwx1022842
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/2/22
 */
@Data
public class ListResourceReq {
    private AppEnv appEnv;

    private Application application;

    private AppResource parentResource;
}