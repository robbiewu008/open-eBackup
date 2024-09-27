/*
 *
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

package openbackup.access.framework.resource.dto;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 传递任务req
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-27
 */
@Setter
@Getter
public class DeliverTaskReq {
    private String taskId;

    private String status;

    private List<AgentDto> agents;

    /**
     * agent信息dto
     */
    @Getter
    @Setter
    public static class AgentDto {
        private String endpoint;
    }
}
