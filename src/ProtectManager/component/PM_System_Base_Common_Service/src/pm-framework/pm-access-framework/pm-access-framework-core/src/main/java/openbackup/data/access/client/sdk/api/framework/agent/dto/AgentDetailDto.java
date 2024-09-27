package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

import java.util.List;

/**
 * Agent Detail Dto
 *
 * @author fwx1022842
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/2/28
 */
@Data
public class AgentDetailDto extends AgentBaseDto {
    private List<AppResource> resourceList;
}
