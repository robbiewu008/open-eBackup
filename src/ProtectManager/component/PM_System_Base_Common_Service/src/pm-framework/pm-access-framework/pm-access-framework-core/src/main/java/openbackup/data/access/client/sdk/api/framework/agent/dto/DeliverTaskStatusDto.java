package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Getter;
import lombok.Setter;

/**
 * 传递任务状态
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-12-22
 */
@Getter
@Setter
public class DeliverTaskStatusDto {
    private String taskId;

    private String status;

    private String script;
}
